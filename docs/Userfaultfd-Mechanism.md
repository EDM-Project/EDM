## How Userfaultfd works and how to use it [1]: 

1. **Create userfaultfd object:**

	```
    uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	```

	The syscall creates an object for delegation of page-fault handling to a user-space
	application, the syscall returns a file descriptor for event polling. 
      
2. **API Handshake:**

	```
    uffdio_api.api = UFFD_API;
    uffdio_api.features = UFFD_FEATURE_EVENT_FORK;
    if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
               errExit("ioctl-UFFDIO_API")
	```

	UFFIDO_API  allows a handshake between the kernel and user space to determine the API version and supported features. 
	An example of a commonly used feature- is UFFD_FEATURE_EVENT_FORK which is one of the non-cooperative userfaultfd features. UFFD_FEATURE_EVENT_FORK enables duplicate userfaultfd object from the parent process into the newly created process.

3. **Register memory address ranges**

	```
    uffdio_register.range.start = (unsigned long) addr;
    uffdio_register.range.len = len;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
        errExit("ioctl-UFFDIO_REGISTER");
	```
	UFFDIO_REGISTER ioctl registers address ranges to be monitored by userfaultfd. 
	Note that userfaultfd registers virtual memory regions that already exist. Before the registration, it validates that the range contains addresses within existing and supporting virtual memory regions. [2]
	
4. **Create a monitor thread that will process the userfaultfd events:**

	The thread sends the event to the relevant handling function.

	```
	//* Loop, handling incoming events on the userfaultfd file descriptor. */
           struct pollfd pollfd;
           pollfd.fd = uffd;
           pollfd.events = POLLIN;

           while (poll(&pollfd, 1, -1) > 0)
           {
               /* Read an event from the userfaultfd. */
               nread = read(uffd, &msg, sizeof(msg));
               ...
               switch (msg.event) {
                   case UFFD_EVENT_PAGEFAULT:
                       handle_page_fault_event(&msg,uffd);
                       break;
                   case UFFD_EVENT_FORK:
                       handle_event_fork(&msg);
                       break;
                       .... 
                }
            }
	```

5. **Resolve page fault**

	```
	unsigned long long vaddr = msg->arg.pagefault.address; // extract address
    /* fill uffdio_copy struct for using UFFDIO_COPY ioctl*/
    struct uffdio_copy uffdio_copy; 
    memcpy(source_page, data, PAGE_SIZE);
    /* Copy the page pointed to by 'source_page' into the faulting region. */
    uffdio_copy.src = (unsigned long) source_page;
    uffdio_copy.dst = (unsigned long) vaddr &
                        ~(PAGE_SIZE - 1);
    uffdio_copy.len = PAGE_SIZE;
    uffdio_copy.mode = 0;
    uffdio_copy.copy = 0;
    if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
        errExit("ioctl-UFFDIO_COPY");
	```

	UFFDIO_COPY atomically copies some existing page contents from userspace.
	After coping and resolving the page fault, the faulting thread wakes up. 
	Another useful ioctl is UFFDIO_ZEROPAGE for atomically copying a zero page.
	
## Userfaultfd in EDM
Except for the ioctls in [DM-Handler](https://github.com/EDM-Project/EDM/wiki/DM-Handler), there are two more places in the project in which userfaultfd features are used:

1. **LD_PRELOAD - mmap symbol**

In the Client side process, LD_PRELOAD was set to a custom shared library of mmap called edm_mmap.so. Therefore it will be loaded before any other library and mmap calls will be redirected to the custom mmap. 
edm_mmap works as regular mmap, but if the specified address (first argument of mmap) is part of the "shared space" - the mapping will be registered to userfaultfd object.

The motivation behind it is to implement a system that will be orthogonal to user code. As mentioned in step 3 above, userfaultfd registers existing memory regions. So to ensure that all memory maps (in the shared space) are managed in userspace, we need to register it just after it is created.

2. **Eviction**

Page evictions are implemented by using mremap() call with the flag MREMAP_DONTUNMAP which remaps a mapping to a new address but does not unmap the mapping at the old address [3]. After mremap call, any access to addresses that "remapped" will result in a page fault will be handled by userfaultfd  (assuming the range was registered).


***
sources and links:

[1] code has taken partially from man page  https://man7.org/linux/man-pages/man2/userfaultfd.2.html

[2] userfaultfd kernel code (validate range function) -https://elixir.bootlin.com/linux/latest/source/fs/userfaultfd.c#L1243  

[3] mremap man page- https://man7.org/linux/man-pages/man2/mremap.2.html

Other information about userfaultfd is based on:
- Linux kernel documentation - https://www.kernel.org/doc/html/latest/admin-guide/mm/userfaultfd.html
***
