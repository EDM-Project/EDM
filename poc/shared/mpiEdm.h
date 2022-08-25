#ifndef MPIEDM_H
#define MPIEDM_H

#include "mpi.h"
#include <stdio.h>
#include <sys/mman.h>
#include "mpiStructs.h"
#include <unistd.h>

#define PAGE_REQUEST_TAG 0
#define EVICT_REQUEST_TAG 1
#define ACK_TAG 2
#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


namespace MPI_EDM {
class MpiEdm {
private:

public:
    int rank;
    MPI_Comm mpi_comm;
    char port_name[MPI_MAX_PORT_NAME];

};

}
#endif
