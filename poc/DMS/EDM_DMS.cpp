#include "EDM_DMS.h"
#include<stdio.h> 
#include<fcntl.h> 
#include <unistd.h>

EDM_DMS::EDM_DMS(int argc, char *argv[]){
    
    start_addr = std::stoi (argv[1],nullptr,16);
    num_of_pages = std::atoi(argv[2]);

    disk_path = "disk";
    mpi_instance = new MPI_EDM::MpiDms(argc,argv);
    dm_tread = std::thread(&EDM_DMS::DmHandlerThread,this);
    xpet_thread = std::thread(&EDM_DMS::XpetThread,this);
    
}

EDM_DMS::~EDM_DMS() {
    
    dm_tread.join();
    xpet_thread.join();
    MPI_Finalize();
    delete mpi_instance;
}
void EDM_DMS::ReadPageFromDisk(uintptr_t addr, char* page){
    int fd = open("disk", O_RDWR | O_CREAT, 0777);
    if (spt.IsAddrExist(addr))
    {
        std::cout << "DMS - read page address: 0x" << addr << " from disk" << std::endl;  

        off_t offset = addr - start_addr;
        pread(fd, page,PAGE_SIZE, offset);

    }
    else{ // addr accessed first time, copy zero page
        std::cout << "DMS - page accessed first time, copy zero page" << std::endl;  
        page = {0};
    }
    close(fd);
}
void EDM_DMS::WritePageTodisk(uintptr_t addr, char* page){
    int fd = open("disk", O_RDWR | O_CREAT , 0777);
    /*offset = TODO : GET START ADDRES AND CALCULATE OFFSET*/
    off_t offset = addr - start_addr;
    if (int res = pwrite(fd, page,PAGE_SIZE,offset) < 0) {
         std::cout<< "DMS - Error writing to disk!" <<std::endl;
    }
    close(fd);
}

void EDM_DMS::HandleRequestGetPage(MPI_EDM::RequestGetPageData* request)
{
   // read from disk page in address , meanwhile simple page
   char* mem  = (char*)malloc(PAGE_SIZE);
   ReadPageFromDisk(request->vaddr,mem);
   memcpy(request->page,mem,PAGE_SIZE);
   free(mem);

}

void EDM_DMS::HandleRequestEvictPage (MPI_EDM::RequestEvictPageData* request) {
   std::cout << "DMS - Handle page eviction in address 0x" << request->vaddr << std::endl;
   // send page to disk
   WritePageTodisk(request->vaddr, request->page);
   

}

void EDM_DMS::DmHandlerThread()
{
   
   for (;;) { 
      
      MPI_EDM::RequestGetPageData page_request = mpi_instance->ListenRequestGetPage();
      std::cout << "DMS - get request to send content of page in addreass: 0x" << page_request.vaddr << std::endl;  
      HandleRequestGetPage(&page_request);
      mpi_instance->SendPageBackToApp(page_request);
      spt.UpdateSPT(page_request.vaddr, INSTANCE_0);
      std::cout << spt << std::endl;
      std::cout << "DMS - page in address: 0x" << page_request.vaddr << " send to app" << std::endl;  

   }

}

void EDM_DMS::XpetThread()
{
   
   for (;;) { 
        MPI_EDM::RequestEvictPageData evict_request = mpi_instance->ListenRequestEvictPage();
        HandleRequestEvictPage(&evict_request);
        std::cout << "DMS - Page evicted successfuly, sending ack" << std::endl;
        mpi_instance->SendAckForEvictPage(evict_request.vaddr);
      /* TODO: update spt the addres current location is in disk */
        spt.UpdateSPT(evict_request.vaddr,DISK);
        std::cout << spt << std::endl;

   }

}