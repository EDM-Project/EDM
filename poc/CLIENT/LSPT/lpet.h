#ifndef LPET_H
#define LPET_H
#include <vector>
#include "page.h"
#include "../../shared/logger.h"

#include "../mpiClient/mpiClient.h"
#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


using std::vector;

class Lpet{

    bool first_run;
    //vector<Page>::iterator start_point;
    int start_point;
    uint32_t low_thresh;
    uint32_t high_thresh;
    MPI_EDM::MpiClient* mpi_instance;
    char* buffer;
    std::mutex& page_list_mutex;



public:

    Lpet(MPI_EDM::MpiClient* mpi_instance, vector<Page>& page_list_ref, int high, int low, std::mutex& page_list_mutex);
    Lpet() = default;
    ~Lpet() = default;
    Lpet& operator=(const Lpet& a);
    vector<Page>& page_list;
    uint32_t run();
    std::string PrintLpetState(); 
    //void RunLpetThread();
    void EraseFromPageList(int index);
    int GetPageListSize();
    std::thread ActivateLpet();

    friend std::ostream& operator<<(std::ostream& os, const Lpet& lpet);
};
std::ostream& operator<<(std::ostream& os, const Lpet& lpet);


#endif