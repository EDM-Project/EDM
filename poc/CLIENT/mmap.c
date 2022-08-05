#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <linux/userfaultfd.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <poll.h>

void* (*orig_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){ 
    
    orig_mmap = dlsym(RTLD_NEXT, "mmap");
    if (orig_mmap == NULL)
	{
	printf("error- ld_preload couldn't find mmap symbol\n");
	}
    void* res = orig_mmap (addr,  length,  prot,  flags,  fd,  offset);
    if (res == MAP_FAILED) {
        printf("mmap - failed to allocate memory\n");
        return MAP_FAILED;
    }

    if (addr > (void*)0x1B58000  && addr < (void*)0x1F40000){
        if (res !=  addr) {
            printf("mmap - failed to allocate user request area");
        }
        long uffd = atol(getenv("uffd"));
        struct uffdio_register uffdio_register;
        uffdio_register.range.start = (unsigned long) addr;
        uffdio_register.range.len = 4096;
        uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
        if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
        perror("ioctl-UFFDIO_REGISTER");
    }
    return res;
}
