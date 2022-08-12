#ifndef EDM_DMS_H
#define EDM_DMS_H


#include "SPT.h"
#include <thread>
#include "../shared/MpiEdm.h"
#include <string>
class EDM_DMS {

private:
uintptr_t start_addr;
std::string disk_path;
SPT spt;
std::thread dm_tread;
std::thread xpet_thread;
MPI_EDM::MpiDms* mpi_instance;
void ParseConfigFile ();
public:

EDM_DMS(int argc, char *argv[]);
~EDM_DMS();
void ReadPageFromDisk(uintptr_t addr,char* page);
void WritePageTodisk(uintptr_t addr, char* page);
void DmHandlerThread();
void XpetThread();
void HandleRequestEvictPage (MPI_EDM::RequestEvictPageData* request);
void HandleRequestGetPage(MPI_EDM::RequestGetPageData* request);


};
#endif