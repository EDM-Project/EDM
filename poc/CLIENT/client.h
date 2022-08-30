#ifndef CLIENT_H
#define CLIENT_H

#include <thread>
#include <vector>
#include <condition_variable> 
#include <algorithm>
#include "../shared/mpiEdm.h"
#include "../shared/logger.h"
#include "LSPT/lpet.h"
#include "LSPT/page.h"
#include "LSPT/lspt.h"
#include "DmHandler/dmHandler.h"


class Client {

private:
    uintptr_t start_addr;
    uintptr_t end_addr;
    int high_threshold;
    int low_threshold;
    Lpet* lpet;
    
    

    
    DmHandler* ufd;
    MPI_EDM::MpiClient* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
    void ParseConfigFile ();

public:
    std::mutex run_lpet_mutex;
    std::condition_variable cv;
    LSPT lspt;    

    //for debug
    bool is_lpet_running;

    Client ();
    ~Client();
    //void AddToPageList(uintptr_t vaddr);
    //void PrintPageList();
    //int GetPageListSize();
    //bool IsPageExist(uintptr_t vaddr);
    void RunLpetThread();
    void WaitForRunLpet();
    void UserThread();
    void RunUserThread();

};
#endif