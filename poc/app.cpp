#include "mpi.h"
#include "shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>


int main(int argc, char *argv[])
{   
   MPI_EDM::MpiApp mpi_instance (argc, argv);
   char mem [3];
   char* addr = (char*) mmap(NULL, 4096*10, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   Userfaultfd ufd (4096*10,addr, &mpi_instance); 
   std::thread dm_handler_thread = ufd.ActivateUserFaultfd();
   
   for (int i = 0; i < 10 ; i++ ) { 
      char x = addr[PAGE_SIZE *i];
   }
   sleep(100);
   dm_handler_thread.join();
   MPI_Finalize();
/* init threads - lpet(idle pages..) ufd */
/* init mpi */ 


/* thread app */ 

   return 0;
}