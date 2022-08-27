#include <iostream>
#include <vector>
#include <thread>
#include "idlePage.h"

using std::vector;
using std::thread;
using std::string;

#define DEBUG_STATUS true
#define MREMAP_DONTUNMAP	4

#include "lpet.h"

Lpet::Lpet(MPI_EDM::MpiClient* mpi_instance, LSPT& lspt, int high, int low) :
    lspt(lspt), high_thresh(high), low_thresh(low), first_run(true)
{
        
         this->start_point = 0;
         this->mpi_instance = mpi_instance;
         buffer =  (char*) mmap(NULL,PAGE_SIZE,PROT_READ | PROT_WRITE , MAP_ANONYMOUS | MAP_PRIVATE ,-1,0);
}


uint32_t Lpet::run()
{
    LOG(DEBUG) << "[Lpet] - start cycle, print current state:";
    LOG(DEBUG) << "[Lpet] - start address: " << PRINT_AS_HEX(lspt.AtIndex(start_point).vaddr);
    LOG(DEBUG) << lspt;
    uint32_t ctr = 0;
    uint32_t evicted_ctr = 0;
        
    int init_size = lspt.GetSize();
    bool first_cycle = true;
    bool is_evicted = false;
    int index = start_point;
    while( lspt.GetSize() > low_thresh)
    {
        std::this_thread::yield();

        is_evicted = false;
        if(first_cycle && ctr >= init_size)
        {
            first_cycle = false;
        }
        if(index == lspt.GetSize())
        {
            index = 0;
        }
        if(DEBUG_STATUS) {LOG(DEBUG) << "[LPET] checking page in addr " << PRINT_AS_HEX(lspt.AtIndex(index).vaddr);}
        if(lspt.AtIndex(index).is_idle() || !first_cycle) // need to evict the current page
        {
            is_evicted = true;
            if(DEBUG_STATUS) {LOG(DEBUG) << "[LPET] removing page in addr " << PRINT_AS_HEX(lspt.AtIndex(index).vaddr);}
            Page evicted = lspt.AtIndex(index);

            char content [PAGE_SIZE];
            char* mem  = (char*)malloc(PAGE_SIZE);
            memcpy(mem,(char*)evicted.vaddr,PAGE_SIZE);

            mpi_instance->RequestEvictPage(evicted.vaddr, mem);
            free(mem);
            mremap((void*)evicted.vaddr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, buffer);

            lspt.Remove(index);
            evicted_ctr++;
        }
        else // no need to evict the current page - raise idle flag to 1'
        {
            lspt.AtIndex(index).set_idle();
        }
        if (!is_evicted){
            index++;
        }
        ctr++;
    }

    this->start_point = index;


    return evicted_ctr;
}


std::thread Lpet::ActivateLpet() { 
    std::thread t (&Lpet::run,this);
    return t;

}
