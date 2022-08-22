#include "userfaultfd.h"
#include <iostream>
#include "../EDM_Client.h"

Userfaultfd::Userfaultfd(MPI_EDM::MpiApp* mpi_instance, EDM_Client* edm_client) {
    this->len = len;
    this->addr = addr;
    this->mpi_instance = mpi_instance;
    struct uffdio_api uffdio_api;
    struct uffdio_register uffdio_register;
    long uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1)
        LOG(ERROR) << "[Userfaultfd] : syscall userfaultfd failed";
    setenv("uffd",std::to_string(uffd).c_str(),1);
    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
        LOG(ERROR) << "[Userfaultfd] : ioctl- UFFDIO_API failed";

    
    this->uffd = uffd;
    this->edm_client = edm_client;
}


void Userfaultfd::ListenPageFaults(){
    static struct uffd_msg msg;   /* Data read from userfaultfd */
    ssize_t nread;

    /* Loop, handling incoming events on the userfaultfd
        file descriptor. */
    
    struct pollfd pollfd;
    pollfd.fd = uffd;
    pollfd.events = POLLIN;

    while (poll(&pollfd, 1, -1) > 0)
    {
        /* Read an event from the userfaultfd. */
        nread = read(uffd, &msg, sizeof(msg));
        if (nread == 0) {
            LOG(ERROR) << " [Userfaultfd] - EOF on userfaultfd! ";
            exit(EXIT_FAILURE);
        }
        if (nread == -1) {
            perror("read");
        }
        switch (msg.event) {
            case UFFD_EVENT_PAGEFAULT:
                HandleMissPageFault(&msg);
                break;
            case UFFD_EVENT_FORK:
                break;
            case UFFD_EVENT_REMAP:
                break;
            case UFFD_FEATURE_EVENT_REMOVE:
                break;
            default:
                break;
        }
    }
}
void Userfaultfd::HandleMissPageFault(struct uffd_msg* msg){
    static char *page = NULL;
    struct uffdio_copy uffdio_copy;

    /* Create a page that will be copied into the faulting region. */

    if (page == NULL) {
        page = (char*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED)
            LOG(ERROR) << "[Userfaultfd] - mmap temp page for copying failed ";
    }
    /* Display info about the page-fault event. */
    LOG(DEBUG) << "[Userfaultfd] - UFFD_EVENT_PAGEFAULT event: \n" <<
     "flags = " << msg->arg.pagefault.flags << "  address = " << PRINT_AS_HEX(msg->arg.pagefault.address) ;
    
    
    //this->edm_client->PrintPageList();
    int evicted_counter = this->edm_client->RunLpet();
    if (evicted_counter != 0) {
        LOG(DEBUG) << "[Userfaultfd] - num of evicted pages : " <<evicted_counter; 
        this->edm_client->PrintPageList();
    }

    MPI_EDM::RequestGetPageData request_page = mpi_instance->RequestPageFromDMS(msg->arg.pagefault.address);
    //LOG(DEBUG) << "[Userfaultfd] - RequestPageFromDMS finished " ;
    memcpy(page,request_page.page, PAGE_SIZE);
    //LOG(DEBUG) << "[Userfaultfd] - page copied to buffer" ;


    //this->edm_client->PrintPageList();

    /* Copy the page pointed to by 'page' into the faulting
        region. */

    uffdio_copy.src = (unsigned long) page;
    /* We need to handle page faults in units of pages(!).
        So, round faulting address down to page boundary. */

    uffdio_copy.dst = (unsigned long) msg->arg.pagefault.address &
                        ~(PAGE_SIZE - 1);
    uffdio_copy.len = PAGE_SIZE;
    uffdio_copy.mode = 0;
    uffdio_copy.copy = 0;
    if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
        LOG(ERROR) << "[Userfaultfd] - ioctl UFFDIO_COPY failed";

    
    //LOG(DEBUG) << "[Userfaultfd] - page copy completed - line109" ;
    this->edm_client->AddToPageList(msg->arg.pagefault.address);
    //LOG(DEBUG) << "[Userfaultfd] - page added to page list" ;

    LOG(DEBUG) << "[Userfaultfd] - uffdio_copy.copy returned " << uffdio_copy.copy ;
}
std::thread Userfaultfd::ActivateDM_Handler(){
    std::thread t (&Userfaultfd::ListenPageFaults,this);
    return t;
}
