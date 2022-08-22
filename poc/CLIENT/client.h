#ifndef CLIENT_H
#define CLIENT_H

#include <thread>
#include <vector>
#include "../shared/mpiEdm.h"
#include "../shared/logger.h"
#include "LSPT/lpet.h"
#include "LSPT/page.h"
#include "uffd/uffd.h"


class Client {

private:
    uintptr_t start_addr;
    uintptr_t end_addr;
    int high_threshold;
    int low_threshold;
    std::vector<Page> page_list;
    Lpet* lpet;
    
    Uffd* ufd;
    MPI_EDM::MpiApp* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
    void ParseConfigFile ();

public:
    Client ();
    ~Client();
    void AddToPageList(uintptr_t vaddr);
    void PrintPageList();
    int RunLpet();
    void UserThread();
    void RunUserThread();

};
#endif