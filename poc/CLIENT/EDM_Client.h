#ifndef EDM_CLIENT_H
#define EDM_CLIENT_H

#include <thread>
#include <vector>
#include "../shared/MpiEdm.h"
#include "../shared/CPPLogger.h"
#include "LSPT/Lpet.h"
#include "LSPT/Page.h"
#include "userfaultfd/userfaultfd.h"


class EDM_Client {

private:
    uintptr_t start_addr;
    uintptr_t end_addr;
    int high_threshold;
    int low_threshold;
    std::vector<Page> page_list;
    Lpet* lpet;
    
    Userfaultfd* ufd;
    MPI_EDM::MpiApp* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
    void ParseConfigFile ();

public:
    EDM_Client ();
    ~EDM_Client();
    void AddToPageList(uintptr_t vaddr);
    void PrintPageList();
    int RunLpet();
    void UserThread();
    void RunUserThread();

};
#endif