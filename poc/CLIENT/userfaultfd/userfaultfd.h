#ifndef USERFAULTFD_H
#define USERFAULTFD_H

#define _GNU_SOURCE
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
#include "../../shared/MpiEdm.h"

#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec

#define PAGE_SIZE 4096
class EDM_Client;

class Userfaultfd {
    private:

    long uffd;          /* userfaultfd file descriptor */
    char *addr;         /* Start of region handled by userfaultfd */
    uint64_t len;       /* Length of region handled by userfaultfd */
    std::thread thr;      /* ID of thread that handles page faults */
    MPI_EDM::MpiApp* mpi_instance;
    EDM_Client* edm_client; 

    public:
    Userfaultfd() = default;
    Userfaultfd(MPI_EDM::MpiApp* mpi_instance, EDM_Client* edm_client);
    ~Userfaultfd() = default;
    void ListenPageFaults();
    void HandleMissPageFault(struct uffd_msg* msg);
    std::thread ActivateDM_Handler();

};
#endif