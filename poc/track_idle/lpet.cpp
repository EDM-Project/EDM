// // #include "idle_page.c"
// // #include "monitor_pages.cpp"
// #include <iostream>
// #include <vector>
// #include <thread>

// using std::vector;
// using std::thread;

// #define DEBUG_STATUS true

#include "lpet.h"

//-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-= Page Implementation -=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=

Page::Page(uint32_t vaddr) : vaddr(vaddr) {};

bool Page::is_idle()
{
    //TODO
    
    if(vaddr == 106)
    {
        return true;
    }
    return false;
}

//-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-= Lpet Implementation -=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=

Lpet::Lpet(vector<Page>& page_list_ref, int high, int low) : 
        page_list(page_list_ref), high_thresh(high), low_thresh(low), first_run(true)
    {
        start_point = vector<Page>::iterator();
    }
    
Lpet& Lpet::operator=(const Lpet& a)
{
    this->page_list = a.page_list;
    this->low_thresh = a.low_thresh;
    this->high_thresh = a.high_thresh;
    this->start_point = a.start_point;
    return *this;
}
    
    
int Lpet::run()
{
    if(start_point == vector<Page>::iterator())
    {
        if(page_list.begin() == vector<Page>::iterator())
        {
            return 0;
        }
        start_point = page_list.begin();
    }
    if(this->first_run)
    {
        start_point = page_list.begin();
        this->first_run = false;
    }
    int ctr = 0;
    int evicted_ctr = 0;
    if(page_list.size() >= high_thresh) //need to evict pages
    {
        int init_size = page_list.size();
        bool first_cycle = true;
        vector<Page>::iterator it = start_point;
        while(page_list.size() > low_thresh)
        {
            if(first_cycle && ctr >= init_size)
            {
                first_cycle = false;
            }
            if(it == page_list.end())
            {
                it = page_list.begin();
            }
            if(DEBUG_STATUS) {std::cout << "checking page in addr " << (*it).vaddr << std::endl;}
            if((*it).is_idle() || !first_cycle)
            {
                if(DEBUG_STATUS) {std::cout << "removing page in addr " << (*it).vaddr << std::endl;}
                it-- = page_list.erase(it);
                evicted_ctr++;
            }
            it++;
            ctr++;
        }
        this->start_point = it;
    }
    return evicted_ctr;
}


void fill_vector(vector<Page>& page_list, int start_addr, int end_addr)
{
    for(int i = start_addr ; i < end_addr ; i++)
    {
        page_list.push_back(Page(i));
    }
}

// App::App(int high_thresh, int low_thresh) : lpet(Lpet(page_list, high_thresh, low_thresh)), page_list(vector<Page>()) {}

App::App(int high_thresh, int low_thresh) : page_list(vector<Page>()), lpet(Lpet(page_list, high_thresh, low_thresh)) {}


// App::App(int high_thresh, int low_thresh) : lpet(Lpet(page_list, high_thresh, low_thresh))
// {
//     page_list = vector<Page>();        
//     fill_vector(page_list, 100, 110);
//     this->lpet = Lpet(page_list, high_thresh, low_thresh);
// }
    
int App::lp_evict()
{
    return lpet.run();
}


// //-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-= main -=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=

// int main()
// {
//     App app = App(10, 5);
//     bool debug = DEBUG_STATUS;
    
//     if(debug) {std::cout << "num of objects in list: " << app.page_list.size() << std::endl;}
    
//     int evicted = app.lp_evict();
    
//     if(debug) {std::cout << "num of evicted pages: " << evicted << std::endl;}
//     if(debug) {std::cout << "num of objects in list: " << app.page_list.size() << std::endl;}
    
//     fill_vector(app.page_list, 110, 129);
    
//     if(debug) {std::cout << "num of objects in list: " << app.page_list.size() << std::endl;}
    
//     int evicted_2 = app.lp_evict();
    
//     if(debug) {std::cout << "num of evicted pages: " << evicted_2 << std::endl;}
//     if(debug) {std::cout << "num of objects in list: " << app.page_list.size() << std::endl;}
    
//     return 0;
// }