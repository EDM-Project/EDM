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

Lpet::Lpet(MPI_EDM::MpiClient* mpi_instance, vector<Page>& page_list_ref, int high, int low, std::mutex& page_list_mutex) :
    page_list(page_list_ref), high_thresh(high), low_thresh(low), first_run(true), page_list_mutex(page_list_mutex)
{
        
         this->start_point = 0;
         this->mpi_instance = mpi_instance;
         buffer =  (char*) mmap(NULL,PAGE_SIZE,PROT_READ | PROT_WRITE , MAP_ANONYMOUS | MAP_PRIVATE ,-1,0);
}

Lpet& Lpet::operator=(const Lpet& a)
{
    this->page_list = a.page_list;
    this->low_thresh = a.low_thresh;
    this->high_thresh = a.high_thresh;
    this->start_point = a.start_point;
    return *this;
}


uint32_t Lpet::run()
{
    
    // if(page_list.size() == 0)
    // {
    //     if(DEBUG_STATUS) {LOG(DEBUG) << "[Lpet] PAGE LIST EMPTY " ;}
    //     return 0;
    // }

    uint32_t ctr = 0;
    uint32_t evicted_ctr = 0;
    // if(page_list.size() >= high_thresh) //need to evict pages
    // {
        
        LOG(DEBUG) << *this;
        int init_size = page_list.size();
        bool first_cycle = true;
        bool is_evicted = false;
        int index = start_point;
        while(page_list.size() > low_thresh)
        {
            std::lock_guard<std::mutex> lockGuard(this->page_list_mutex);
            is_evicted = false;
            if(first_cycle && ctr >= init_size)
            {
                first_cycle = false;
            }
            if(index  == page_list.size())
            {
                index = 0;
            }
            if(DEBUG_STATUS) {LOG(DEBUG) << "[LPET] checking page in addr " << PRINT_AS_HEX(page_list[index].vaddr);}
            if(page_list[index].is_idle() || !first_cycle) // need to evict the current page
            {
                is_evicted = true;
                if(DEBUG_STATUS) {LOG(DEBUG) << "[LPET] removing page in addr " << PRINT_AS_HEX(page_list[index].vaddr);}
                Page evicted = page_list[index];

                char content [PAGE_SIZE];
                char* mem  = (char*)malloc(PAGE_SIZE);
                memcpy(mem,(char*)evicted.vaddr,PAGE_SIZE);

                mpi_instance->RequestEvictPage(evicted.vaddr, mem);
                free(mem);
                mremap((void*)evicted.vaddr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, buffer);

                page_list.erase(page_list.begin() + index);
                evicted_ctr++;
            }
            else // no need to evict the current page - raise idle flag to 1'
            {
                page_list[index].set_idle();
            }
            if (!is_evicted){
                index++;
            }
            ctr++;
        }

        this->start_point = index;

    //}
    return evicted_ctr;
}



std::ostream& operator<<(std::ostream& os, const Lpet& lpet){
  
    os << "[Lpet] - Print Lpet State" <<std::endl;
    os << "Lpet start point: "  << PRINT_AS_HEX(lpet.page_list[lpet.start_point].vaddr) << std::endl;
    os << "page list state: " <<std::endl;
    os << "-----START LSPT ----" << std::endl;
    for (auto& it: lpet.page_list) {
        os << it;
    }
    os << "-----END LSPT ----" << std::endl;

    os << std::endl;
}

