#include "mpi.h"
#include "../shared/MpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>
#include "EDM_DMS.h"
#include <thread>  

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{  
   
   EDM_DMS edm_dms(argc,argv);
   return 0;
}