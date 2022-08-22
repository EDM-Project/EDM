#ifndef MPISTRUCTS_H
#define MPISTRUCTS_H

#include <stdint.h>
#include "mpi.h"

#define PAGE_SIZE 4096
#define FLAGS_SIZE 10
#define ERROR_SIZE 10
namespace MPI_EDM {
typedef struct 
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
    char flags [FLAGS_SIZE];
} RequestGetPageData ;

typedef struct 
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
} RequestEvictPageData;


typedef struct {
    uintptr_t vaddr;
    char error [ERROR_SIZE];
} AckPage;

MPI_Datatype GetMPIDataType(RequestGetPageData request_page);

MPI_Datatype GetMPIDataType(RequestEvictPageData request) ;

MPI_Datatype GetMPIDataType(AckPage ack);
  
}
#endif