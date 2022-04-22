#include "mpi.h"
#include <stdio.h>
#include <pthread.h>
#define PAGE_REQUEST 0
#define EVICT_REQUEST 1
#define ACK 2

void* dm_handler_service(void * arg)
{
   MPI_Comm app = (MPI_Comm)(arg);
   for (;;) { 
      int req;
      printf("DMS- wait to receive req of page\n");
      MPI_Recv(&req, 1, MPI_INT, 0, 0, app, MPI_STATUS_IGNORE);
      printf("DMS - i got %d req \n", req);
      int res = req+1;
      printf("server - send response \n");
      MPI_Send(&res, 1, MPI_INT, 0, 0, app);
      
   }

}
int main(int argc, char *argv[])
{  
   MPI_Init(&argc, &argv);

   // Open port.
   char port_name[MPI_MAX_PORT_NAME];
   MPI_Open_port(MPI_INFO_NULL, port_name);

   // Publish port name and accept client.
   MPI_Comm app;
   MPI_Publish_name("dms", MPI_INFO_NULL, port_name);
   MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &app);
   

   pthread_t pt;
   pthread_create(&pt, NULL, dm_handler_service, (void *) app);
   pthread_join(&pt, NULL); 

   MPI_Unpublish_name("name", MPI_INFO_NULL, port_name);

   MPI_Finalize();
   return 0;
}