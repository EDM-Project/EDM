#include "mpi.h"
#include "shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "EDM_Client.h"
int main(int argc, char *argv[])
{   
   EDM_Client edm_client(argc,argv);
   edm_client.Init();
   edm_client.RunUserThread();

   return 0;
}