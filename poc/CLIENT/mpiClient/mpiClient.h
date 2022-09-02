#ifndef MPICLIENT_H
#define MPICLIENT_H

#include "../../shared/mpiEdm.h"
#include "../../shared/logger.h"
namespace MPI_EDM{
class MpiClient : public MpiEdm {

private:

public:
    MpiClient();
    RequestGetPageData RequestPageFromDMS (uintptr_t vaddr);
    void RequestEvictPage (uintptr_t vaddr, char* page);
    
};
}
#endif