#include "mpi.h"

int main(int argc, char *argv[])
{   
   /*
   current run line: mpirun -n 1 manager 0x235000 10 > log.txt
   */
   MPI_Init(&argc, &argv);

   MPI_Comm intercomm;
   // Spawn two applications with a single process for each application.
   // Server must be spawned before client otherwise the client will complain at MPI_Lookup_name().
   MPI_Comm_spawn("dms", argv +1 , 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
   MPI_Comm_spawn("app", argv +1 , 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
   
   MPI_Finalize();
   return 0;
}