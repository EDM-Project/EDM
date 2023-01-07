#include "syscallInjectors.h"



unsigned long injectUffdCreate(pid_t pid) {
        struct ptrace_do* target = ptrace_do_init(pid);
		long uffd = ptrace_do_syscall(target, __NR_userfaultfd,  O_NONBLOCK, 0, 0, 0, 0, 0);

        LOG(DEBUG) << "injectUffdCreate: uffd in son process is: " << uffd;
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
    LOG(DEBUG)<< "duplicateFileDescriptor : the duplicate fd is : " << duplicatedFd;
    return duplicatedFd;
}

void injectUffdRegister(pid_t pid, int fd, uintptr_t start_address, uintptr_t end_address) { 
    
        LOG(DEBUG) << "injectUffdRegister try to inject in pid: " <<pid;
        struct ptrace_do* target = ptrace_do_init(pid);
        LOG(INFO) << " UFFDIO_REGISTER - start_address: " << convertToHexRep(start_address) << " end_address: " << convertToHexRep(end_address);
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
    LOG(DEBUG) << "injectMmap in len:" << len;
    struct ptrace_do* target = ptrace_do_init(pid);
    char* addr = (char *) ptrace_do_malloc(target, len);
    LOG(DEBUG) << "injectMmap ended";
    ptrace_do_cleanup(target);
    return addr;
}
void injectMremapPage(pid_t pid, uintptr_t addr,uintptr_t dest) { 
    struct ptrace_do* target = ptrace_do_init(pid);
	int res = ptrace_do_syscall(target, __NR_mremap, addr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, dest, 0);
    ptrace_do_cleanup(target);
}

void injectCloseFd(pid_t pid, int fd) { 
    struct ptrace_do* target = ptrace_do_init(pid);
	int res = ptrace_do_syscall(target, __NR_close,fd,0,0,0,0,0);
    ptrace_do_cleanup(target);
}

void readPageFromProcess(pid_t pid, uintptr_t address, char* buffer, size_t page_size) { 
    struct iovec local[1];
    struct iovec remote[1];
    ssize_t nread;

    local[0].iov_base = buffer;
    local[0].iov_len = page_size;
    remote[0].iov_base = (void *) address;
    remote[0].iov_len = page_size;

    nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread != page_size) { 
        LOG(ERROR) << "failed to read page from address" << convertToHexRep(address);
    }
    
}
