#include <stdint.h>
#include "mpi.h"

#define PAGE_SIZE 4096
#define FLAGS_SIZE 10

namespace MPI_EDM {
typedef struct 
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
    char flags [FLAGS_SIZE];
} RequestPageData ;

struct requst_evict_page_t
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
};


struct ack_page_saved_t
{
    uintptr_t vaddr;
    char error [FLAGS_SIZE];
};

MPI_Datatype GetMPIDataType(RequestPageData request_page) {
    MPI_Datatype MPI_RequestPage;
    int lengths[3] = { 8, PAGE_SIZE, FLAGS_SIZE };
 
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    MPI_Get_address(&request_page, &base_address);
    MPI_Get_address(&request_page.vaddr, &displacements[0]);
    MPI_Get_address(&request_page.page[0], &displacements[1]);
    MPI_Get_address(&request_page.flags[0], &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
 
    MPI_Datatype types[3] = { MPI_INT, MPI_CHAR, MPI_CHAR };
    MPI_Type_create_struct(3, lengths, displacements, types, &MPI_RequestPage); 
    MPI_Type_commit(&MPI_RequestPage);
    return MPI_RequestPage;

}
  
}