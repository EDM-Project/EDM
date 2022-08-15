#include "EDM_DMS.h"
#include<stdio.h> 
#include<fcntl.h> 
#include <unistd.h>
#include <fstream>

EDM_DMS::EDM_DMS(int argc, char *argv[]){
    
    disk_path = "disk";
    ParseConfigFile();
    mpi_instance = new MPI_EDM::MpiDms(argc,argv);
    dm_tread = std::thread(&EDM_DMS::DmHandlerThread,this);
    xpet_thread = std::thread(&EDM_DMS::XpetThread,this);
    
}
void EDM_DMS::ParseConfigFile () {
    std::ifstream ReadFile("DMS/dms_config.txt");
	if(!ReadFile.is_open()) {
		std::cerr << "Error: Could not open file.\n";
	}
	std::string word;
	while(ReadFile >> word) {
		if(word == "start_addr") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->start_addr  = std::stoi (word.c_str(),nullptr,16);
		}
	}
	ReadFile.close();
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
        LOG(DEBUG) << "[DMS] - read page address: " << PRINT_AS_HEX(addr) << " from disk";  

        off_t offset = addr - start_addr;
        pread(fd, page,PAGE_SIZE, offset);

    }
    else{ // addr accessed first time, copy zero page
        LOG(DEBUG) << "[DMS] - page accessed first time, copy zero page" ;  
        page = {0};
    }
    close(fd);
}
void EDM_DMS::WritePageTodisk(uintptr_t addr, char* page){
    int fd = open("disk", O_RDWR | O_CREAT , 0777);
    /*offset = TODO : GET START ADDRES AND CALCULATE OFFSET*/
    off_t offset = addr - start_addr;
    if (int res = pwrite(fd, page,PAGE_SIZE,offset) < 0) {
         LOG(DEBUG)<< "[DMS] - Error writing to disk!" ;
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
   LOG(DEBUG) << "[DMS] - Handle page eviction in address " << request->vaddr ;
   // send page to disk
   WritePageTodisk(request->vaddr, request->page);
   

}

void EDM_DMS::DmHandlerThread()
{
   
   for (;;) { 
      
      MPI_EDM::RequestGetPageData page_request = mpi_instance->ListenRequestGetPage();
      LOG(DEBUG) << "[DMS] - get request to send content of page in addreass: " << PRINT_AS_HEX(page_request.vaddr) ;  
      HandleRequestGetPage(&page_request);
      mpi_instance->SendPageBackToApp(page_request);
      spt.UpdateSPT(page_request.vaddr, INSTANCE_0);
      LOG(DEBUG) << spt ;
      LOG(DEBUG) << "[DMS] - page in address: " << PRINT_AS_HEX(page_request.vaddr) << " send to app" ; 

   }

}

void EDM_DMS::XpetThread()
{
   
   for (;;) { 
        MPI_EDM::RequestEvictPageData evict_request = mpi_instance->ListenRequestEvictPage();
        HandleRequestEvictPage(&evict_request);
        LOG(DEBUG) << "[DMS] - Page evicted successfuly, sending ack" ;
        mpi_instance->SendAckForEvictPage(evict_request.vaddr);
      /* TODO: update spt the addres current location is in disk */
        spt.UpdateSPT(evict_request.vaddr,DISK);
        LOG(DEBUG) << spt;

   }

}