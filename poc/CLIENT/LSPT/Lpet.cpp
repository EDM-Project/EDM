// #include "monitor_pages.cpp"
#include <iostream>
#include <vector>
#include <thread>
#include "idle_page.h"

using std::vector;
using std::thread;
using std::string;

#define DEBUG_STATUS true
#define MREMAP_DONTUNMAP	4

#include "Lpet.h"

/*
Lpet::Lpet(vector<Page>& page_list_ref, int high, int low) :
        page_list(page_list_ref), high_thresh(high), low_thresh(low), first_run(true)
{
    start_point = page_list.begin();

}
*/

Lpet::Lpet(MPI_EDM::MpiApp* mpi_instance, vector<Page>& page_list_ref, int high, int low) :
    page_list(page_list_ref), high_thresh(high), low_thresh(low), first_run(true)
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
    //if(this->first_run)
    //{
        if(page_list.size() == 0)
        {
            if(DEBUG_STATUS) {LOG(DEBUG) << "PAGE LIST EMPTY " ;}
            return 0;
        }
        //this->start_point = page_list.begin();
        LOG(DEBUG) << "[LPET run - 53 ] start point is : "<< PRINT_AS_HEX(page_list[start_point].vaddr); 

        //this->first_run = false;
   // }
    uint32_t ctr = 0;
    uint32_t evicted_ctr = 0;
    //LOG(DEBUG) << "[LPET] page list size: " << page_list.size() << " high tresh: " << high_thresh;
    if(page_list.size() >= high_thresh) //need to evict pages
    {
        LOG(DEBUG) << "[LPET run - 60 ] start point is : "<< PRINT_AS_HEX(page_list[start_point].vaddr); 
        int init_size = page_list.size();
        bool first_cycle = true;
        bool is_evicted = false;
        int index = start_point;
        //vector<Page>::iterator it = this->start_point;
        while(page_list.size() > low_thresh)
        {
            is_evicted = false;
            if(first_cycle && ctr >= init_size)
            {
                first_cycle = false;
            }
            if(index  == page_list.size())
            {
                index = 0;
            }
            if(DEBUG_STATUS) {LOG(DEBUG) << "checking page in addr " << PRINT_AS_HEX(page_list[index].vaddr);}
            if(page_list[index].is_idle() || !first_cycle) // need to evict the current page
            {
                is_evicted = true;
                if(DEBUG_STATUS) {LOG(DEBUG) << "removing page in addr " << PRINT_AS_HEX(page_list[index].vaddr);}
                Page evicted = page_list[index];

                char content [PAGE_SIZE];
                char* mem  = (char*)malloc(PAGE_SIZE);
                memcpy(mem,(char*)evicted.vaddr,PAGE_SIZE);

                // for(int i = 0 ; i < PAGE_SIZE ; i++)
                // {
                //     mem[i] = char((evicted.vaddr) + i);
                // }
                //evicted.capture();
                string request_page = mpi_instance->RequestEvictPage(evicted.vaddr, mem);
                free(mem);
                mremap((void*)evicted.vaddr,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, buffer);

                page_list.erase(page_list.begin() + index);
                //index++;
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
        
       // LOG(DEBUG) << "[LPET run - 109 ] start point is : "<< PRINT_AS_HEX((*(this->start_point)).vaddr); 

        this->start_point = index;
        //LOG(DEBUG) << "[LPET run - 112 ] start point is : "<< PRINT_AS_HEX((*(this->start_point)).vaddr); 

    }
    return evicted_ctr;
}
/*

void fill_vector(vector<Page>& page_list, int start_addr, int end_addr)
{
    char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE*NUM_OF_PAGES, PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    uintptr_t addr;
    for(int i = 0 ; i < NUM_OF_PAGES ; i++)
    {
        addr = uintptr_t(area_1) + i*PAGE_SIZE;
        area_1[i*PAGE_SIZE] = 'x';
        page_list.push_back(Page(addr));
    }
}

void print_table(vector<Page> table)
{
    std::cout << "-=-=-=-=-=-=-=-=-=--=Page State-=-=-=-=-=-=-=-=-=--=" << std::endl;
    for(auto& it: table)
    {
        std::cout << it.vaddr << ' ' << it.pfn << ' ' << it.is_idle() << ' ' << std::endl;
    }
    std::cout << "-=-=-=-=-=-=-=-=-=--= Page State - End -=-=-=-=-=-=-=-=-=--=" << std::endl;
}

*/
/*
// //-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-= main -=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=

int main()
{

    vector<Page> vec = vector<Page>();
    Lpet lpet = Lpet(vec, 10, 5);
    fill_vector(lpet.page_list, 0, 10);

    bool debug = DEBUG_STATUS;

    if(debug) {std::cout << "num of objects in list: " << lpet.page_list.size() << std::endl;}
    print_table(lpet.page_list);

    int evicted = lpet.run();
    print_table(lpet.page_list);

    // if(debug) {std::cout << "num of evicted pages: " << evicted << std::endl;}
    // if(debug) {std::cout << "num of objects in list: " << lpet.page_list.size() << std::endl;}

    // fill_vector(lpet.page_list, 100, 200);
    // print_table(lpet.page_list);

    // if(debug) {std::cout << "num of objects in list: " << lpet.page_list.size() << std::endl;}

    // int evicted_2 = lpet.run();
    // print_table(lpet.page_list);

    // if(debug) {std::cout << "num of evicted pages: " << evicted_2 << std::endl;}
    // if(debug) {std::cout << "num of objects in list: " << lpet.page_list.size() << std::endl;}

    return 0;
}
*/              
