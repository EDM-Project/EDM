#ifndef EDM_CLIENT_H
#define EDM_CLIENT_H

#include "shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <thread>
#define NUM_OF_PAGES 10
class EDM_Client {

private:
    char* start_addr;
    Userfaultfd* ufd;
    MPI_EDM::MpiApp* mpi_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
public:
    EDM_Client (int argc, char *argv[]);
    ~EDM_Client();
    void Init ();
    void Dispose();
    void UserThread();
    void RunUserThread();

};
#endif