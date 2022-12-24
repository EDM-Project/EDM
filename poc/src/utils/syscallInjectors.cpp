#include "syscallInjectors.h"

unsigned long remoteUffd(pid_t pid) {
        struct ptrace_do* target = ptrace_do_init(pid);
		long uffd = ptrace_do_syscall(target, __NR_userfaultfd,  O_NONBLOCK, 0, 0, 0, 0, 0);

        struct uffdio_api uffdio_api;
		
        uffdio_api.api = UFFD_API;
    	uffdio_api.features = 0;
        char* addr = (char *) ptrace_do_malloc(target, sizeof(uffdio_api));
        memcpy(addr,&uffdio_api,sizeof(struct uffdio_api));
        void* temp_addr = ptrace_do_push_mem(target, addr);
		int res = ptrace_do_syscall(target, __NR_ioctl, uffd,UFFDIO_API, (unsigned long)temp_addr, 0, 0, 0);
        if (res < 0) 
        {
            printf ("UFFDIO_API errno is %d\n",errno);
        }
    	LOG(DEBUG)<< "UFFDIO_API ioctl result is " << res;
       
        ptrace_do_cleanup(target);
        return uffd;
}

int duplicateFileDescriptor(pid_t pid, int fd) { 
    int pidfd = syscall(SYS_pidfd_open, pid, 0);
    int duplicatedFd = syscall(__NR_pidfd_getfd, pidfd, fd, 0);
    if (duplicatedFd < 0) 
    {
        LOG(ERROR) << "__NR_pidfd_getfd errno is " << errno;
    
    }
    return duplicatedFd;
}

void injectUffdRegister(pid_t pid, int fd, uint64_t start_address, uint64_t end_address) { 
    
        struct ptrace_do* target = ptrace_do_init(pid);
        LOG(INFO) << " UFFDIO_REGISTER - start_address" << convertToHexRep(start_address) << " end_address" << convertToHexRep(end_address);
    	struct uffdio_register uffdio_register;
        
        uffdio_register.range.start = (unsigned long)start_address;
		uffdio_register.range.len = (end_address - start_address);
		uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

        char* addr_reg = (char *) ptrace_do_malloc(target, sizeof(struct uffdio_register));
        
        memcpy(addr_reg,&uffdio_register,sizeof(struct uffdio_register));
        void* temp_addr_reg = ptrace_do_push_mem(target, addr_reg);

		int res = ptrace_do_syscall(target, __NR_ioctl, fd,UFFDIO_REGISTER, (unsigned long)temp_addr_reg, 0, 0, 0);
		LOG(DEBUG) << "UFFDIO_REGISTER ioctl result is " << res;
        ptrace_do_cleanup(target);

}

char* injectMmap(pid_t pid, size_t len) { 
    struct ptrace_do* target = ptrace_do_init(pid);
    char* addr = (char *) ptrace_do_malloc(target, len);
    ptrace_do_cleanup(target);
    return addr;
}
void injectMremapPage(pid_t pid, uint64_t addr,uint64_t dest) { 
    struct ptrace_do* target = ptrace_do_init(pid);
	int res = ptrace_do_syscall(target, __NR_mremap, addr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, dest, 0);
    ptrace_do_cleanup(target);
}

void injectCloseFd(pid_t pid, int fd) { 
    struct ptrace_do* target = ptrace_do_init(pid);
	int res = ptrace_do_syscall(target, __NR_close,fd,0,0,0,0,0);
    ptrace_do_cleanup(target);
}
