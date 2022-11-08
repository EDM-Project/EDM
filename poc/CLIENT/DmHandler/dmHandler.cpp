#include <iostream>
#include "dmHandler.h"
#include "../client.h"

#define DEBUG_MODE 1

DmHandler::DmHandler(sw::redis::Redis* redis_instance, Client* client, int high_threshold, int low_threshold) {
    this->len = len;
    this->addr = addr;
    this->high_threshold = high_threshold;
    this->low_threshold = low_threshold;
    this->redis_instance = redis_instance;
    struct uffdio_api uffdio_api;
    struct uffdio_register uffdio_register;
    long uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
    if (uffd == -1)
        LOG(ERROR) << "[DmHandler] : syscall userfaultfd failed";
    setenv("uffd",std::to_string(uffd).c_str(),1);
    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
        LOG(ERROR) << "[DmHandler] : ioctl- UFFDIO_API failed";

    auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
    this->uffd = uffd;
    this->client = client;
}


void DmHandler::ListenPageFaults(){
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
            LOG(ERROR) << " [DmHandler] - EOF on userfaultfd! ";
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
void DmHandler::HandleMissPageFault(struct uffd_msg* msg){

    /* Display info about the page-fault event. */
    LOG(INFO) << "\n--------------START HADNLING PAGE FAULT--------------";
    LOG(INFO) << "[DmHandler] - UFFD_EVENT_PAGEFAULT in address = " << PRINT_AS_HEX(msg->arg.pagefault.address) ;
    
    unsigned long long vaddr = msg->arg.pagefault.address;
    
    //handle race condition with lpet
    while(this->client->lspt.IsPageExist(vaddr)) {
        //busy wait. should be very short period of time
    }

    
    InvokeLpetIfNeeded();

    //for debug, wait until lpet done 
    if (DEBUG_MODE) {
        while(this->client->is_lpet_running) {}
    }
    LOG(INFO) << "[DmHandler] - send request for the page in address " << PRINT_AS_HEX(vaddr) << " from DMS";
    /* thinking about error handling approach, thus:*/
    try {
        auto request_page = this->redis_instance->get(std::to_string(vaddr)); /* conversion should be ok*/
        if (request_page) /* key exists*/ {
            LOG(INFO) << "[DmHandler] - received ack for page in address : " << PRINT_AS_HEX(vaddr) << " (previously accessed)" ; 
            LOG(INFO) << "[DmHandler] - copying page content from DMS to address : " << PRINT_AS_HEX(vaddr);
           CopyExistingPage(vaddr,request_page.page);
        }
        else { /* key does not exist*/
            LOG(INFO) << "[DmHandler] - received ack for page in address : " << PRINT_AS_HEX(vaddr) << " (first access)" ; 
            LOG(INFO) << "[DmHandler] - copying zero page to address : " << PRINT_AS_HEX(vaddr);
            CopyZeroPage(vaddr);
        }
    }
    
    catch (...) {   /* an error has occured - catch any type of error*/  
        LOG(ERROR) << "[DmHandler] - failed to resolve page fault for address " <<  PRINT_AS_HEX(vaddr) ;

    }
    /*MPI_EDM::RequestGetPageData request_page = mpi_instance->RequestPageFromDMS(vaddr);*/
    
    this->client->lspt.Add(vaddr);
}

void DmHandler::InvokeLpetIfNeeded(){ 
    std::unique_lock<std::mutex> lck(this->client->run_lpet_mutex);
    if (this->client->lspt.GetSize() >= high_threshold ) {
        LOG(INFO) << "[DmHandler] - reached high threshold, waking up lpet" ;
        this->client->cv.notify_all();
    }
    //avoid memory flood
    while (this->client->lspt.GetSize() >= high_threshold){
        LOG(DEBUG) << "[DmHandler] - avoid memory flood- dmhandler goes to sleep";
        this->client->cv.wait(lck);
    }
}


void DmHandler::CopyZeroPage(uintptr_t vaddr){
    struct uffdio_zeropage uffdio_zero;

    uffdio_zero.range.start =  (unsigned long) vaddr &
                        ~(PAGE_SIZE - 1);
    uffdio_zero.range.len = PAGE_SIZE;
    uffdio_zero.mode = 0;
    LOG(INFO) << "\n--------------FINISH HADNLING PAGE FAULT--------------\n\n";

    if (ioctl(uffd, UFFDIO_ZEROPAGE, &uffdio_zero) == -1)
        LOG(ERROR) << "[DmHandler] - ioctl UFFDIO_ZEROPAGE failed";
    
}
void DmHandler::CopyExistingPage(uintptr_t vaddr,char* source_page_content){

    static char *page = NULL;
    /* Create a page that will be copied into the faulting region. */

    if (page == NULL) {
        page = (char*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED)
            LOG(ERROR) << "[DmHandler] - mmap temp page for copying failed ";
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
    LOG(INFO) << "\n--------------FINISH HADNLING PAGE FAULT--------------\n\n";
    if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
        LOG(ERROR) << "[DmHandler] - ioctl UFFDIO_COPY failed";
}


std::thread DmHandler::ActivateDM_Handler(){
    std::thread t (&DmHandler::ListenPageFaults,this);
    return t;
}
