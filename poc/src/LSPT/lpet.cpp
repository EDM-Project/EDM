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

Lpet::Lpet(pid_t pid,sw::redis::Redis* redis_instance, LSPT& lspt, int high, int low) :
    pid(pid), lspt(lspt), high_thresh(high), low_thresh(low), first_run(true)
{
        
         this->start_point = 0;
         this->redis_instance = redis_instance;
         buffer =  injectMmap(pid,PAGE_SIZE);
}


uint32_t Lpet::run()
{
    LOG(INFO) << "\n-----------------START LPET-----------------\n";
    LOG(INFO) << "[Lpet] - start address: " << PRINT_AS_HEX(lspt.AtIndex(start_point).vaddr);
    LOG(INFO) << lspt;
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
        LOG(INFO) << "[LPET] checking if page in addr " << PRINT_AS_HEX(lspt.AtIndex(index).vaddr) << " is idle";
        if(lspt.AtIndex(index).is_idle() || !first_cycle) // need to evict the current page
        {
            is_evicted = true;
            LOG(INFO) << "[LPET] sending eviction request for page in addr " << PRINT_AS_HEX(lspt.AtIndex(index).vaddr);
            Page evicted = lspt.AtIndex(index);
            
            char* mem  = (char*)malloc(PAGE_SIZE);
            
            //mremap((void*)evicted.vaddr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, buffer);
            injectMremapPage(pid, evicted.vaddr, (uintptr_t) buffer);

            readPageFromProcess(pid, (uintptr_t) buffer, mem);
            //memcpy(mem,buffer,PAGE_SIZE);

            std::string str_vaddr = convertToHexRep(evicted.vaddr);
            this->redis_instance->set(str_vaddr,mem); /* update command using Redis server*/
            LOG(INFO) << "[Lpet] - received ack for eviction of page in address : " << PRINT_AS_HEX(evicted.vaddr)  ; 
           
            free(mem);
            

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
    LOG(INFO) << "\n-----------------END LPET-----------------\n";

    return evicted_ctr;
}


std::thread Lpet::ActivateLpet() { 
    std::thread t (&Lpet::run,this);
    return t;

}
