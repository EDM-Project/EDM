#include "mpi.h"
#include "../shared/mpiEdm.h"
#include "../shared/logger.h"
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
   LOG(DEBUG) << "[DMS] dms init start running";
   DMS dms(argc,argv);
   return 0;
}