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
   int size_of_area;
   uintptr_t start_area;
   if (argc > 2) {
      start_area = std::stoi (argv[1],nullptr,16);
      size_of_area = std::atoi(argv[2]);
   }else{
      exit(0);
   }

   EDM_Client edm_client(argc,argv);
   edm_client.Init();
   edm_client.RunUserThread();

   return 0;
}