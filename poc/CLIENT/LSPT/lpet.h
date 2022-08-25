#ifndef LPET_H
#define LPET_H
#include <vector>
#include "page.h"
#include "../../shared/logger.h"

#include "../../shared/mpiEdm.h"
#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


using std::vector;

class Lpet{

    bool first_run;
    //vector<Page>::iterator start_point;
    int start_point;
    uint32_t low_thresh;
    uint32_t high_thresh;
    MPI_EDM::MpiApp* mpi_instance;
    char* buffer;



public:

    Lpet(MPI_EDM::MpiApp* mpi_instance, vector<Page>& page_list_ref, int high, int low);
    Lpet() = default;
    ~Lpet() = default;
    Lpet& operator=(const Lpet& a);
    vector<Page>& page_list;
    uint32_t run();
    std::string PrintLpetState(); 
};


#endif