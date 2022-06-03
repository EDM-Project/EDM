#include "mpi.h"
#include "../shared/MpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>
#include "EDM_DMS.h"
#include <thread>  



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

   EDM_DMS edm_dms(argc,argv);
   return 0;
}