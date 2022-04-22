#include "mpi.h"
#include <stdio.h>

#define PAGE_REQUEST 0
#define EVICT_REQUEST 1
#define ACK 2



int main(int argc, char *argv[])
{   
   MPI_Init(&argc, &argv);

   // Look up for server's port name.
   char port_name[MPI_MAX_PORT_NAME];
   MPI_Lookup_name("dms", MPI_INFO_NULL, port_name);
   
   // Connect to server.
   MPI_Comm dms;
   /*
   MPI_Comm_connect establishes communication with a server specified by port_name.
   It is collective over the calling communicator and returns an intercommunicator in which the remote group participated in an MPI_Comm_accept.
   The MPI_Comm_connect call must only be called after the MPI_Comm_accept call has been made by the MPI job acting as the server.
   If the named port does not exist (or has been closed), MPI_Comm_connect raises an error of class MPI_ERR_PORT.
   MPI provides no guarantee of fairness in servicing connection attempts.
   That is, connection attempts are not necessarily satisfied in the order in which they were initiated, 
   and competition from other connection attempts may prevent a particular connection attempt from being satisfied.
   The port_name parameter is the address of the server. It must be the same as the name returned by MPI_Open_port on the server.
   */
   MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &dms);


   // Send data to server.
    
   printf("dm- send req %d to dms \n",PAGE_REQUEST);
   int req = PAGE_REQUEST;
   MPI_Send(&req, 1, MPI_INT, 0, 0, dms);
   printf("client- req sent\n");
   int res=0; 
   MPI_Recv(&res, 1, MPI_INT, 0, 0, dms, MPI_STATUS_IGNORE);
   printf("client- i got response of %d\n",res);

   MPI_Finalize();
   return 0;
}