## What is DM-Handler 

DM-Handler is an event-based mechanism for handling page faults in userspace and for memory management.

## Why use userspace page management in EDM project: 

Page fault handling is normally the kernel's responsibility. When a process attempts to access an address that is not currently mapped to a location in RAM, the kernel responds by mapping a page to that location and, if needed, filling that page with data from secondary storage. [1]
In EDM project, we try to split the application state among multiple storage units that are not accessible from kernel mode.
Therefore, Userspace page fault handling allows us to fetch pages from remote servers, databases, etc., and solve this problem. 


## DM-Handler goals 

- be the "gateway" of shared memory pages that can be distributed on instances in cloud or memory units.
-  allow easy implementation of on-demand paging and future eviction. 
	
DM-Handler is based on Userfaultfd, which is the core behind it.
Userfaultfd is a mechanism for resolving page faults and tracking virtual memory layout in userspace. 
Due to being an essential part of DM-Handler, we will provide a brief overview of the userfaultfd usage : [How Userfaultfd works and how to use it](https://github.com/EDM-Project/EDM/wiki/Userfaultfd-Mechanism)


## DM-Handler functionality

###  DM-Handler thread: 

monitoring thread which polls(wait for events on file descriptor) userfaultfd, then reads the message event and sends it to the corresponding handling function.
Currently, there is one handling function that handles page miss faults. 

###  Function for handling miss page:
Performs the following steps:

1. Parse event message.

2. Validate that the faulting page doesn't exist in LSPT (if it does exist- the page was recently chosen by lpet thread for eviction. The page is currently in the process of eviction, so dmhandler has to wait).

3. Invoke lpet thread for evicting pages if LSPT reached the high threshold.

4. Send a request to DMS for the page, with RequestPageFromDMS.

5. According to RequestPageFromDMS status:
	i. If the page is "new" (this is the first access to it) - use ioctl  UFFDIO_ZEROPAGE 
	ii. If the page was evicted before- use ioctl UFFDIO_COPY
Each of this ioctl eventually wakes up the faulting thread. 
		
6. Page fault resolved.
		
Top view -how DM-Handler interacts with other modules : [flow - resolving page fault](https://github.com/EDM-Project/EDM/wiki/Flow---Handling-Page-Fault)

***

**Sources:**

[1] https://lwn.net/Articles/550555/ -  User-space page fault handling
