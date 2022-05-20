#include "mpi.h"
#include "shared/MpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>

#include <thread>         // std::thread
#define PAGE_REQUEST 0
#define EVICT_REQUEST 1
#define ACK 2

using namespace MPI_EDM;


void DmHandlerThread(MpiDms* mpi_instance)
{
   
   for (;;) { 
      
      RequestPageData page_request = mpi_instance->ListenRequestPage();
      std::cout << "DMS - get request to handle address: 0x" << page_request.vaddr << std::endl;  
      mpi_instance->HandleRequestPage(page_request);
      std::cout << "DMS - page in address: 0x" << page_request.vaddr << " send to app" << std::endl;  

   }

}


int main(int argc, char *argv[])
{  
   
   MpiDms mpiInstance = MpiDms(argc, argv);
   printf("DMS- my rank is %d \n",mpiInstance.rank);


   std::thread pt (DmHandlerThread, &mpiInstance);

   pt.join(); 

   MPI_Unpublish_name("name", MPI_INFO_NULL, mpiInstance.port_name);

   MPI_Finalize();
   return 0;
}