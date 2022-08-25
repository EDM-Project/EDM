#ifndef MPISTRUCTS_H
#define MPISTRUCTS_H

#include <stdint.h>
#include "mpi.h"

#define PAGE_SIZE 4096
#define INFO_SIZE 4
#define ERROR_SIZE 128
namespace MPI_EDM {

enum request_page_status { new_page, error, existing_page};
enum request_evict_page_status {success, fail};

typedef struct 
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
    int info;
} RequestGetPageData ;

typedef struct 
{
    uintptr_t vaddr;
    char page [PAGE_SIZE];
    int info;
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