
![image](https://user-images.githubusercontent.com/63795552/187938640-66398665-519f-4884-804a-467aed287bd9.png)


## **Instance Side**

* Instance - a virtual machine in the cloud.

* Process - a process which runs under the instance's operating system.

* App - user-code, the code of the user who pays for the cloud computing service

* Local SPT - a module which every instance has is its local Stored Page Table (SPT). This table contains the virtual 
 addresses of the pages that are part of the app's memory, and that their content is located in the instance's memory.

* DM (Disc management) handler -a module in the instance side, which controls the communication with DMS in case a page-fault is thrown

* Lpet - low priority eviction thread - wakes up when the instance's memory got to a high threshold, and is responsible to evict the least-recently used pages from instance back to DMS

## **DMS side**
	
* DMS (Disc management server) - a software on an independent machine (not an instance, in the sense that user-code would not be run on it). DMS is responsible for managing the distributed memory. 

* SPT (stored page table) -  holds the current memory state on all machines (for example a given address is currently at instance #3).

* Dml - (Disc management listener) - a module whose role is to listen to page-faults thrown in the instances, and return the a page's content back.

* XPET (External low priority eviction thread) - a module whose role is to listen to eviction requests from the instances and store pages in disk.

* Disc - a memory under DMS in which it stores all evicted pages, and returns them back to the instances upon request.
