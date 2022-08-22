#include "mpi.h"
#include "../shared/mpiEdm.h"

#include <stdio.h>
#include <sys/mman.h>
#include "dms.h"
#include <thread>  

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{  
   
   DMS dms(argc,argv);
   return 0;
}