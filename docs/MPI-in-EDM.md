## What is MPI and Why using it in EDM
From wikipedia: 

Message Passing Interface (MPI) is a standardized and portable [message-passing](https://en.wikipedia.org/wiki/Message-passing) standard designed to function on parallel computing architectures.

In EDM-Project we used open-mpi-4 which is an open-source MPI implementation [2]. MPI goals are high performance, scalability, and portability and became the "de facto standard" for communication among processes that model a parallel program running on a distributed memory system. Therefore, MPI was chosen as the suitable message fabric for EDM-Project. 

## MPI usage in EDM

In EDM-Project, the communication between DMS  and the Client side (DM-handler & lpet) is performed by MPI. 
There are 3 custom typed messages:
	
### 1. RequestGetPageData: 

Contains 3 fields:
- vaddr - virtual address of the requested page.
- Page- the content of the page in vaddr.
- Info - request status. The values are enum typed -  { new_page, error, existing_page} 
**Usage:**

Dm-handler send RequestGetPageData to DMS, and specify vaddr as the virtual address of the faulting page.
- If the page exists in DMS's SPT, DMS copies page content to "page" and sets info=existing_page
- If the page doesn't exist in DMS's SPT, DMS sets info=new_page
- In any case of error, DMS sets info=error.
	
### 2. RequestEvictPageData:
Contains 3 fields:
- vaddr - virtual address of the requested page.
- Page- the content of the page in vaddr.
- Info - request status. The values are enum typed -  { success, fail} 

**Usage**:
lpet send RequestEvictPageDatato DMS, and specify "vaddr" as the virtual address of the page and its content in "page".
- If DMS managed to store page in the disc, it sets info=success
- In any case of error, DMS sets info=fail.
	
### 3. AckPage:
Contains 3 fields:
- vaddr - virtual address.
- error- buffer for storing error 

**Usage:**
When it is necessary to use communication with ACK, an informative error can be specified in the "error" field.
	
	

***

**Sources:**

[1] - https://en.wikipedia.org/wiki/Message_Passing_Interface - wiki page of MPI

[2] - https://github.com/open-mpi/ompi - open-mpi github repository