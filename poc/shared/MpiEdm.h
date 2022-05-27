#ifndef MPIEDM_H
#define MPIEDM_H

#include "mpi.h"
#include <stdio.h>
#include <sys/mman.h>
#include "MpiStructs.h"
#include <unistd.h>

#define PAGE_REQUEST_TAG 0
#define EVICT_REQUEST_TAG 1
#define ACK_TAG 2

namespace MPI_EDM {
class MpiEdm {
private:

public:
    int rank;
    MPI_Comm CommApp;
    char port_name[MPI_MAX_PORT_NAME];

};

class MpiDms : public MpiEdm {
private:
    
public:
    
    MpiDms (int argc, char *argv[]);
    RequestGetPageData ListenRequestGetPage ();
    void SendPageBackToApp (RequestGetPageData response);
    RequestEvictPageData ListenRequestEvictPage();
    void SendAckForEvictPage(uintptr_t vaddr);

};

class MpiApp : public MpiEdm {

private:

public:

    MpiApp(int argc, char *argv[]);
    RequestGetPageData RequestPageFromDMS (uintptr_t vaddr);
    std::string RequestEvictPage (uintptr_t vaddr, char* page);
    
};
}
#endif
