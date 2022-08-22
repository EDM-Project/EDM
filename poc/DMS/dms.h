#ifndef DMS_H
#define DMS_H
#include <thread>
#include <string>

#include "spt.h"
#include "../shared/logger.h"
#include "../shared/mpiEdm.h"

class DMS {

private:
uintptr_t start_addr;
std::string disk_path;
SPT spt;
std::thread dm_tread;
std::thread xpet_thread;
MPI_EDM::MpiDms* mpi_instance;
void ParseConfigFile ();
public:

DMS(int argc, char *argv[]);
~DMS();
void ReadPageFromDisk(uintptr_t addr,char* page);
void WritePageTodisk(uintptr_t addr, char* page);
void DmHandlerThread();
void XpetThread();
void HandleRequestEvictPage (MPI_EDM::RequestEvictPageData* request);
void HandleRequestGetPage(MPI_EDM::RequestGetPageData* request);


};
#endif