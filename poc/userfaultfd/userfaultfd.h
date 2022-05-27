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
#include "../shared/MpiEdm.h"

#define PAGE_SIZE 4096

class Userfaultfd {
    private:

    long uffd;          /* userfaultfd file descriptor */
    char *addr;         /* Start of region handled by userfaultfd */
    uint64_t len;       /* Length of region handled by userfaultfd */
    std::thread thr;      /* ID of thread that handles page faults */
    MPI_EDM::MpiApp* mpi_instance;
    public:

    Userfaultfd(uint64_t len, char* addr, MPI_EDM::MpiApp* mpi_instance);
    ~Userfaultfd() = default;
    void ListenPageFaults();
    void HandleMissPageFault(struct uffd_msg* msg);
    std::thread ActivateUserFaultfd();

};
#endif