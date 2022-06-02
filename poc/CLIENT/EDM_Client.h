#ifndef EDM_CLIENT_H
#define EDM_CLIENT_H

#include "../shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <thread>
#include <vector>

class EDM_Client {

private:
    uintptr_t start_addr;
    int num_of_pages;
    std::vector<uintptr_t> pages_list;
    Userfaultfd* ufd;
    MPI_EDM::MpiApp* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
public:
    EDM_Client (int argc, char *argv[]);
    ~EDM_Client();
    void Init ();
    void AddToPageList(uintptr_t addr);
    void UserThread();
    void RunUserThread();

};
#endif