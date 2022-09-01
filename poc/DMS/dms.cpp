#include "dms.h"
#include <stdio.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <fstream>

DMS::DMS(int argc, char *argv[]){
    
    disk_path = "disk";
    ParseConfigFile();
    mpi_instance = new MPI_EDM::MpiDms(argc,argv);
    dm_tread = std::thread(&DMS::ServeDmHandlerRequests,this);
    xpet_thread = std::thread(&DMS::XpetThread,this);
    LOG(DEBUG) << "[DMS] - DMS ready for serving requests";
    
}
void DMS::ParseConfigFile () {
    std::ifstream ReadFile("DMS/dms_config.txt");
	if(!ReadFile.is_open()) {
		LOG(ERROR) << "Error: Could not open file.\n";
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


DMS::~DMS() {
    
    dm_tread.join();
    xpet_thread.join();
    MPI_Finalize();
    delete mpi_instance;
}
void DMS::ReadPageFromDisk(uintptr_t addr, char* page, int* info){
    int fd = open("disk", O_RDWR | O_CREAT, 0777);
    if (spt.IsAddrExist(addr))
    {
        LOG(DEBUG) << "[DMS] - read page address: " << PRINT_AS_HEX(addr) << " from disk";  

        off_t offset = addr - start_addr;
        if (pread(fd, page,PAGE_SIZE, offset) != PAGE_SIZE) {
            *info = (int)MPI_EDM::error;
            LOG(ERROR) << "[DMS] - failed to fetch page content in address " << PRINT_AS_HEX(addr);
        }
        else{
            *info = (int)MPI_EDM::existing_page;
        }

    }
    else{ // addr accessed first time, copy zero page
        LOG(DEBUG) << "[DMS] - page accessed first time, set info - new_page" ; 
        *info = (int)MPI_EDM::new_page;
    }
    close(fd);
}
void DMS::WritePageTodisk(uintptr_t addr, char* page, int* info){
    int fd = open("disk", O_RDWR | O_CREAT , 0777);
    /*offset = TODO : GET START ADDRES AND CALCULATE OFFSET*/
    off_t offset = addr - start_addr;
    if (int res = pwrite(fd, page,PAGE_SIZE,offset) < 0) {
         *info = (int)MPI_EDM::fail;
         LOG(ERROR)<< "[DMS] - Error writing page in address " << PRINT_AS_HEX(addr) << "to disk" ;
    }
    else{
         *info = (int)MPI_EDM::success;
         LOG(INFO)<< "[DMS] - page in address " << PRINT_AS_HEX(addr) << " stored in disk" ;
    }
    close(fd);
}

void DMS::HandleRequestGetPage(MPI_EDM::RequestGetPageData* request)
{
   char* mem  = (char*)malloc(PAGE_SIZE);
   ReadPageFromDisk(request->vaddr,mem, &(request->info));
   if (request->info == MPI_EDM::existing_page){
        memcpy(request->page,mem,PAGE_SIZE);
   } //else - page is new or failed to fetch from disc 
   free(mem);
}

void DMS::HandleRequestEvictPage (MPI_EDM::RequestEvictPageData* request) {
   LOG(DEBUG) << "[DMS] - Handle page eviction in address " << PRINT_AS_HEX(request->vaddr) ;
   // store page in disk
   WritePageTodisk(request->vaddr, request->page, &request->info);

}

void DMS::ServeDmHandlerRequests()
{
   
   for (;;) { 
      
      MPI_EDM::RequestGetPageData page_request = mpi_instance->ListenRequestGetPage();
      LOG(INFO) << "[DMS] - get request to send content of page in address: " << PRINT_AS_HEX(page_request.vaddr) ;  
      HandleRequestGetPage(&page_request);
      mpi_instance->SendPageBackToApp(page_request);
      spt.UpdateSPT(page_request.vaddr, INSTANCE_0);
      LOG(DEBUG) << "[DMS] - SPT updated. Current state: ";
      LOG(INFO) << spt ;
      LOG(DEBUG) << "[DMS] - page in address: " << PRINT_AS_HEX(page_request.vaddr) << " sent to app" ; 

   }

}

void DMS::XpetThread()
{
   
   for (;;) { 
        MPI_EDM::RequestEvictPageData evict_request = mpi_instance->ListenRequestEvictPage();
        HandleRequestEvictPage(&evict_request);
        LOG(DEBUG) << "[DMS] - Page evicted successfuly, sending ack" ;
        mpi_instance->SendAckForEvictPage(evict_request.vaddr, MPI_EDM::request_evict_page_status(evict_request.info));
        spt.UpdateSPT(evict_request.vaddr,DISK);
        LOG(DEBUG) << "[DMS] - SPT updated. Current state: ";
        LOG(INFO) << spt;

   }

}