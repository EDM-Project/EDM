## What is idle_page?

idle_page is a bit related to the PFN in the hardware, which marks a PFN as idle when bit is raised to 1'. The bit was introduced to linux kernel to have an interface to determine which address spaces a given workload utilizes, and so dynamic memory management optimizations can be made. According to linux kernel manual a page is considered idle if it has not been accessed since it was marked idle. When idle_page flag is raised to 1' - page is idle, meaning that the workload hasn't stored / loaded to it recently (from the last time it was marked idle).


## What Purpose does idle_page Serve?

A major issue in EDM caching is to be able to prioritize which page should be evicted, without kernel's ability to read directly from page table. For that goal we utilize idle-page from user-space.
 

## How do we use idle_page in EDM?

In the simple case:

1. Raise idle_page flag to 1'
2. Execute workload
3. Check relevant PFN's idle_page bit:
    * If bit is 1' - workload hasn't touched it from the last time we raised it to 1' => Page is idle
    *  Else - workload touched it since last time => Page is hot

What actually happens in EDM is that LPET executes this check as part of it's logic. For more info visit [LPET](https://github.com/EDM-Project/EDM/wiki/LPET).

## How to read and write to idle_page flag?
 
The flags of all PFNs are stored in /sys/kernel/mm/page_idle/bitmap and are accessed by offset determined by PFN. For more info take a look at get_idle_flags function in poc/CLIENT/LSPT/idlePage.cpp.
 
For more info on the flag please visit [linux kernel documentation](https://www.kernel.org/doc/html/latest/admin-guide/mm/idle_page_tracking.html).
 
## Configuration Requirements
1. Using page_idle flag requires the following in kernel: CONFIG_IDLE_PAGE_TRACKING=y
2. Opening /sys/kernel/mm/page_idle/bitmap requires root privileges (sudo mpirun)
3. Since the idle memory tracking feature is based on the memory reclaimer logic, it only works with pages that are on an LRU list (active or inactive).
4. Only accesses to user memory pages are tracked.