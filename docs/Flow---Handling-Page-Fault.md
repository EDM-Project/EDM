## Top view - resolving page fault in EDM

Sequence diagram to show the interactions between modules in sequential order.

![diagram](https://user-images.githubusercontent.com/62066172/188267663-3fa68228-309c-4c9d-ab8c-7fcc6f6afb58.png)
steps:
1. App accesses page that its present bit in PTE is 0, which causes to page fault.
2. mm start handle_mm_fault(). The following functions are called: handle_mm_fault() -> __handle_mm_fault()-> handle_pte_fault()->do_anonymous_page() which detect that page belong to userfaultfd  and calls handle_userfaultfd()
3. handle_userfaultfd() function builds the event message and writes it to userfaultfd object(file descriptor).
4. handle_userfaultfd() make the event visible to userspace. 
5.  DM_Handler, which polls userfaultfd, reads the message and starts handling page fault. It sends a request to DMS via MPI. 
6. if the page exists in SPT, DMS fetches the page from the disc 
7. DMS gets the page data from the disc
8. DMS sets the request status = existing_page.
9. else, the page is new,  DMS sets the request status= new_page.
10. DMS updates SPT - the up-to-date location of the page is the instance that sends the request.
11. DMS sends the request structure via MPI back to DM-handler.
12. According to the status, if new_page is new, DM-Handler calls ioctl UFFDIO_ZEROPAGE() (the function in the kernel is userfaultfd_zeropage()).
13. if the status is existing_page, DM-Handler calls ioctl UFFDIO_COPY() (the function in the kernel is userfaultfd_copy()).
14. DM-Handler add the page to LSPT (with its pfn)
15. Userfaultfd wakes up the faulting thread. The page fault was resolved and the app can continue running.


## Notes:
* The diagram is divided into user space and kernel space. The userfaultfd participant in the diagram refers to userfaultfd mechanism in the kernel. DM-Handler refers to the monitoring thread that calls to ioctls (of userfaultfd).

* The diagram doesn't contain the validations, connections, error handlings, etc., and is meant to explain the big picture of the flow of the interaction between modules in EDM.