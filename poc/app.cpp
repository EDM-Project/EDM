#include "mpi.h"
#include "shared/MpiEdm.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace MPI_EDM;


int main(int argc, char *argv[])
{   
   MpiApp mpiInstance = MpiApp(argc, argv);
    
   RequestGetPageData response =  mpiInstance.RequestPageFromDMS(0x343434);
   char page_to_evict [PAGE_SIZE];
   memcpy (page_to_evict, response.page, PAGE_SIZE );
   std::string error_str = mpiInstance.RequestEvictPage(response.vaddr, page_to_evict);
   std::cout << "APP - ack error is " << error_str << std::endl;
   std::cout << "APP - I FINISHED, bayush" <<  std::endl;
   sleep(100);
   MPI_Finalize();
   return 0;
}