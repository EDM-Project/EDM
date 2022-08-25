#ifndef MPIDMS_H
#define MPIDMS_H

#include "../../shared/mpiEdm.h"
#include "../../shared/logger.h"
namespace MPI_EDM {
class MpiDms : public MpiEdm {
private:
    
public:
    
    MpiDms (int argc, char *argv[]);
    RequestGetPageData ListenRequestGetPage ();
    void SendPageBackToApp (RequestGetPageData response);
    RequestEvictPageData ListenRequestEvictPage();
    void SendAckForEvictPage(uintptr_t vaddr,  MPI_EDM::request_evict_page_status status);

};
}
#endif