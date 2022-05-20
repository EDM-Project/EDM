#include "mpi.h"
#include "shared/MpiEdm.h"
#include <stdio.h>
#include <sys/mman.h>

#define PAGE_REQUEST 0
#define EVICT_REQUEST 1
#define ACK 2
using namespace MPI_EDM;


int main(int argc, char *argv[])
{   
   MpiApp mpiInstance = MpiApp(argc, argv);
    
   RequestPageData response =  mpiInstance.RequestPageFromDMS(0x343434);
   for(int i =0; i < 10 ; i++) {
      std::cout<< "DM - char num " << i << " is " << response.page[i] << std::endl;
   }

   MPI_Finalize();
   return 0;
}