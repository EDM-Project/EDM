#include "mpi.h"
#include "shared/MpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>

#include <thread>  


using namespace MPI_EDM;

void HandleRequestGetPage(RequestGetPageData* request)
{
   std::cout << "DMS - handle page in address: 0x" << request->vaddr << std::endl;  

   // read from disk page in address , meanwhile simple page
   char* mem  = (char*) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   for (int i =0; i< PAGE_SIZE ; i++) {
      mem[i] = 'b';
   }
   memcpy(request->page,mem,PAGE_SIZE);

}

void HandleRequestEvictPage (RequestEvictPageData* request) {
   std::cout << "DMS - Handle page eviction in address 0x" << request->vaddr << std::endl;
   // send page to disk

}

void DmHandlerThread(MpiDms* mpi_instance)
{
   
   for (;;) { 
      
      RequestGetPageData page_request = mpi_instance->ListenRequestGetPage();
      std::cout << "DMS - get request to handle address: 0x" << page_request.vaddr << std::endl;  
      HandleRequestGetPage(&page_request);
      mpi_instance->SendPageBackToApp(page_request);
      std::cout << "DMS - page in address: 0x" << page_request.vaddr << " send to app" << std::endl;  

   }

}

void XpetThread(MpiDms* mpi_instance)
{
   
   for (;;) { 
      std::cout << "DMS - Waiting for page eviction request..." << std::endl;
      RequestEvictPageData evict_request = mpi_instance->ListenRequestEvictPage();
      HandleRequestEvictPage(&evict_request);
      std::cout << "DMS - Page evicted successfuly, sending ack" << std::endl;
      mpi_instance->SendAckForEvictPage(evict_request.vaddr);

   }

}


int main(int argc, char *argv[])
{  
   
   MpiDms mpiInstance = MpiDms(argc, argv);

   std::thread dm_tread (DmHandlerThread, &mpiInstance);
   std::thread xpet_thread (XpetThread, &mpiInstance);

   dm_tread.join(); 
   xpet_thread.join(); 

   MPI_Unpublish_name("name", MPI_INFO_NULL, mpiInstance.port_name);

   MPI_Finalize();
   return 0;
}