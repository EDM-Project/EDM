#include "mpi.h"
#include "../shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "EDM_Client.h"
#include <iostream>
int main(int argc, char *argv[])
{   
   /*
      expection 2 arguments:
      start_area = start address of the virtual mapping
      size_of_area = num of pages 
   */
   printf("LD_PRELOAD : %s\n", getenv("LD_PRELOAD"));
   int size_of_area;
   uintptr_t start_area;
   if (argc > 2) {
      start_area = std::stoi (argv[1],nullptr,16);
      size_of_area = std::atoi(argv[2]);
   }else{
      exit(0);
   }
   EDM_Client edm_client(argc,argv);
   edm_client.RunUserThread();

   return 0;
}