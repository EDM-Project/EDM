#include "mpi.h"
#include "shared/MpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>

#include <thread>  


using namespace MPI_EDM;


void DmHandlerThread(MpiDms* mpi_instance)
{
   
   for (;;) { 
      
      RequestGetPageData page_request = mpi_instance->ListenRequestPage();
      std::cout << "DMS - get request to handle address: 0x" << page_request.vaddr << std::endl;  
      mpi_instance->HandleRequestGetPage(page_request);
      std::cout << "DMS - page in address: 0x" << page_request.vaddr << " send to app" << std::endl;  

   }

}

void XpetThread(MpiDms* mpi_instance)
{
   
   for (;;) { 
      mpi_instance->HandleRequestEvictPage();
   }

}


int main(int argc, char *argv[])
{  
   
   MpiDms mpiInstance = MpiDms(argc, argv);
   printf("DMS- my rank is %d \n",mpiInstance.rank);


   std::thread dm_tread (DmHandlerThread, &mpiInstance);
   std::thread xpet_thread (XpetThread, &mpiInstance);

   dm_tread.join(); 
   xpet_thread.join(); 

   MPI_Unpublish_name("name", MPI_INFO_NULL, mpiInstance.port_name);

   MPI_Finalize();
   return 0;
}