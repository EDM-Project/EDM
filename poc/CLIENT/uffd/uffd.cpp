#include <iostream>
#include "uffd.h"
#include "../client.h"

#define NUM_OF_RETRIES 3

Uffd::Uffd(MPI_EDM::MpiApp* mpi_instance, Client* client) {
    this->len = len;
    this->addr = addr;
    this->mpi_instance = mpi_instance;
    struct uffdio_api uffdio_api;
    struct uffdio_register uffdio_register;
    long uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1)
        LOG(ERROR) << "[Uffd] : syscall userfaultfd failed";
    setenv("uffd",std::to_string(uffd).c_str(),1);
    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
        LOG(ERROR) << "[Uffd] : ioctl- UFFDIO_API failed";

    
    this->uffd = uffd;
    this->client = client;
}


void Uffd::ListenPageFaults(){
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
            LOG(ERROR) << " [Uffd] - EOF on userfaultfd! ";
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
void Uffd::HandleMissPageFault(struct uffd_msg* msg){
    static char *page = NULL;
    struct uffdio_copy uffdio_copy;
    struct uffdio_zeropage uffdio_zero;
    
    /* Display info about the page-fault event. */
    LOG(DEBUG) << "[Uffd] - UFFD_EVENT_PAGEFAULT event: \n" <<
     "flags = " << msg->arg.pagefault.flags << "  address = " << PRINT_AS_HEX(msg->arg.pagefault.address) ;
    
    unsigned long long vaddr = msg->arg.pagefault.address;
    int evicted_counter = this->client->RunLpet();
    if (evicted_counter != 0) {
        LOG(DEBUG) << "[Uffd] - num of evicted pages : " <<evicted_counter; 
        this->client->PrintPageList();
    }
    MPI_EDM::RequestGetPageData request_page;
    for (int i=0; i< NUM_OF_RETRIES; i++) {
        request_page = mpi_instance->RequestPageFromDMS(vaddr);
        if (request_page.info == MPI_EDM::error){
            LOG(ERROR) << "[Uffd] - RequestPageFromDMS failed. Retry " ;
        }
        else{
            LOG(ERROR) << "[Uffd] - RequestPageFromDMS succeeded. " ;
            break;
        }

    }
    switch (request_page.info){
        case(MPI_EDM::error):
            LOG(ERROR) << "[Uffd] - failed to resolve page fault for address " <<  PRINT_AS_HEX(vaddr) ;
        break;
        case (MPI_EDM::new_page):
            LOG(ERROR) << "[Uffd] - page accessed first time, copy zero page of address " << PRINT_AS_HEX(vaddr) ; 
            CopyZeroPage(vaddr);
            /*
            uffdio_zero.range.start =  (unsigned long) vaddr &
                                ~(PAGE_SIZE - 1);
            uffdio_zero.range.len = PAGE_SIZE;
            uffdio_zero.mode = 0;

            if (ioctl(uffd, UFFDIO_ZEROPAGE, &uffdio_zero) == -1)
                LOG(ERROR) << "[Uffd] - ioctl UFFDIO_ZEROPAGE failed";
            */
        break;
        case (MPI_EDM::existing_page):
            LOG(ERROR) << "[Uffd] - page evicted before, copy page content from dms"; 
            CopyExistingPage(vaddr,request_page.page);
            // memcpy(page,request_page.page, PAGE_SIZE);
            // /* Copy the page pointed to by 'page' into the faulting
            // region. */
            // uffdio_copy.src = (unsigned long) page;
            // /* We need to handle page faults in units of pages(!).
            //     So, round faulting address down to page boundary. */

            // uffdio_copy.dst = (unsigned long) vaddr &
            //                     ~(PAGE_SIZE - 1);
            // uffdio_copy.len = PAGE_SIZE;
            // uffdio_copy.mode = 0;
            // uffdio_copy.copy = 0;
            // if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
            //     LOG(ERROR) << "[Uffd] - ioctl UFFDIO_COPY failed";

        break;

    }
    
    this->client->AddToPageList(vaddr);
}

void Uffd::CopyZeroPage(uintptr_t vaddr){
    struct uffdio_zeropage uffdio_zero;

    uffdio_zero.range.start =  (unsigned long) vaddr &
                        ~(PAGE_SIZE - 1);
    uffdio_zero.range.len = PAGE_SIZE;
    uffdio_zero.mode = 0;

    if (ioctl(uffd, UFFDIO_ZEROPAGE, &uffdio_zero) == -1)
        LOG(ERROR) << "[Uffd] - ioctl UFFDIO_ZEROPAGE failed";
    
}
void Uffd::CopyExistingPage(uintptr_t vaddr,char* source_page_content){

    static char *page = NULL;
    /* Create a page that will be copied into the faulting region. */

    if (page == NULL) {
        page = (char*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED)
            LOG(ERROR) << "[Uffd] - mmap temp page for copying failed ";
    }
    
    struct uffdio_copy uffdio_copy;
    memcpy(page, source_page_content, PAGE_SIZE);
    /* Copy the page pointed to by 'page' into the faulting
    region. */
    uffdio_copy.src = (unsigned long) page;
    /* We need to handle page faults in units of pages(!).
        So, round faulting address down to page boundary. */

    uffdio_copy.dst = (unsigned long) vaddr &
                        ~(PAGE_SIZE - 1);
    uffdio_copy.len = PAGE_SIZE;
    uffdio_copy.mode = 0;
    uffdio_copy.copy = 0;
    if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
        LOG(ERROR) << "[Uffd] - ioctl UFFDIO_COPY failed";
}


std::thread Uffd::ActivateDM_Handler(){
    std::thread t (&Uffd::ListenPageFaults,this);
    return t;
}
