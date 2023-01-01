#ifndef SYSCALL_INJECTORS
#define SYSCALL_INJECTORS


#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
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
#include <sys/uio.h>

#include "ptrace_do/libptrace_do.h"
#include "helperfunc.h"
#include "logger.h"

#define __NR_pidfd_getfd 438
#define SYS_pidfd_open 434
#define MREMAP_DONTUNMAP	4

unsigned long injectUffdCreate(pid_t pid);
int duplicateFileDescriptor(pid_t pid, int fd);
void injectUffdRegister(pid_t pid, int fd, uintptr_t start_address, uintptr_t end_address);
char* injectMmap(pid_t pid, size_t len);
void injectMremapPage(pid_t pid, uintptr_t addr,uintptr_t dest);
void injectCloseFd(pid_t pid, int fd);

void readPageFromProcess(pid_t pid, uintptr_t address, char* buffer, size_t page_size = PAGE_SIZE);

#endif