#ifndef DMHANDLER_H
#define DMHANDLER_H

#include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <thread>
#include "../utils/logger.h"
#include "../utils/helperfunc.h"
#include "../utils/syscallInjectors.h"
#include <sw/redis++/redis++.h>


#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec

#define PAGE_SIZE 4096
class AppMonitor;

class DmHandler {
    private:

    long uffd;          /* userfaultfd file descriptor */
    long uffd_son;
    char *addr;         /* Start of region handled by userfaultfd */
    int high_threshold;
    int low_threshold;
    uint64_t len;       /* Length of region handled by userfaultfd */
    std::thread thr;      /* ID of thread that handles page faults */
    sw::redis::Redis* redis_instance;
    AppMonitor* client; 
    pid_t pid;

    public:
    DmHandler() = default;
    DmHandler(sw::redis::Redis* redis_instance, AppMonitor* client, int high_threshold, int low_threshold, pid_t pid);
    ~DmHandler() = default;

    long getUffd() { 
        return uffd;
    }
    long getUffdSon() { 
        return uffd_son;
    }
    void ListenPageFaults();
    void HandleMissPageFault(struct uffd_msg* msg);
    void CopyZeroPage(uintptr_t vaddr);
    void CopyExistingPage(uintptr_t vaddr,const char* source_page_content);
    std::thread ActivateDM_Handler();
    void InvokeLpetIfNeeded();

};
#endif