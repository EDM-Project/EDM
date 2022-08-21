#ifndef LPET_H
#define LPET_H
#include <vector>
#include "Page.h"
// #include "../../shared/MpiEdm.h"

using std::vector;

class Lpet{

    bool first_run;
    vector<Page>::iterator start_point;
    uint32_t low_thresh;
    uint32_t high_thresh;
    // MPI_EDM::MpiApp* mpi_instance;

    
    public:
    
    // Lpet(MPI_EDM::MpiApp* mpi_instance, vector<Page>& page_list_ref, int high, int low);
    Lpet(vector<Page>& page_list_ref, int high, int low);
    ~Lpet() = default;
    Lpet& operator=(const Lpet& a);
    vector<Page>& page_list;
    uint32_t run();
};


// class Lpet{

//     bool first_run;
//     vector<Page>::iterator start_point;
//     uint32_t low_thresh;
//     uint32_t high_thresh;
//     MPI_EDM::MpiApp* mpi_instance;

    
//     public:
    
//     Lpet(MPI_EDM::MpiApp* mpi_instance, vector<Page>& page_list_ref, int high, int low);
//     ~Lpet() = default;
//     Lpet& operator=(const Lpet& a);
//     vector<Page>& page_list;
//     uint32_t run();
// };


#endif