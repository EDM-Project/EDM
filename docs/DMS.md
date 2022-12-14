## What is DMS? 
DMS (Disc manager server) is an application for managing pages across instances.

DMS goals:

1.  Maintain page up-to-date locations.
2.  Serve request of instances for getting pages content.
3.  Serve request of instances for backup pages.
4.  Storing pages content in disc/ storing units.

##  DMS Structure 


DMS contains 3 modules, with them, DMS accomplishes these goals:

### SPT

SPT stores for each page, its up-to-date location.
For example:

![image](https://user-images.githubusercontent.com/62066172/188005853-392a9710-7753-4135-a256-99de632beaa5.png)


SPT is based on an unordered map which is an associative container that contains key-value pairs with unique keys. Search, insertion, and removal of elements have average constant-time complexity.

### Dml thread

DML listens for incoming MPI massages [1] of type [RequestGetPageData](https://github.com/EDM-Project/EDM/wiki/MPI-in-EDM#1-requestgetpagedata)  from instances.

DML steps:

1.  If the page is new, i.e., the page virtual address doesn't exist in SPT.  DML sets the request status- new_page.
If the virtual address of the page exists in SPT, it means the page was evicted before and stored in the disc.
DML fetches the page from disc [2], sets request status- existing_page, and copies the page content to the request buffer.

2.  Send the message back to DM-Handler.
3.  Update SPT - the up-to-date location of the page is in the instance that sent the request.

### Xpet thread 

Xpet listens for incoming MPI messages of the type [RequestEvictPageData](https://github.com/EDM-Project/EDM/wiki/MPI-in-EDM#2-requestevictpagedata) from instances.

Xpet steps:

1.  Store page content in the disc.
2.  Send [AckPage  ](https://github.com/EDM-Project/EDM/wiki/MPI-in-EDM#3-ackpage) to the instance so the instance could safely remove the page. If there was an error of any kind, it will be specified in the "error" buffer of AckPage.
3.  Update SPT -  the up-to-date location of the page is Disc.


***


[1] What is MPI and what MPI messages exist in EDM

[2] Right now the disc is implemented by a simple file and uses file system features. Accesses to disc are calculated relative to starting address of the shared virtual space between instances.