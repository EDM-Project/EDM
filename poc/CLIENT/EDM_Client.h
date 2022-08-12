#ifndef EDM_CLIENT_H
#define EDM_CLIENT_H

#include "../shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <thread>
#include <vector>

class EDM_Client {

private:
    uintptr_t start_addr;
    uintptr_t end_addr;
    int high_threshold;
    int low_threshold;
    std::vector<uintptr_t> pages_list;
    Userfaultfd* ufd;
    MPI_EDM::MpiApp* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
    void ParseConfigFile ();

public:
    EDM_Client ();
    ~EDM_Client();
    void AddToPageList(uintptr_t addr);
    void UserThread();
    void RunUserThread();

};
#endif