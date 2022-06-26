// #include "idle_page.c"
// #include "monitor_pages.cpp"
#include <iostream>
#include <vector>
#include <thread>

using std::vector;
using std::thread;

#define DEBUG_STATUS true


class Page
{
    public:
    Page(uint32_t vaddr);
    uint32_t vaddr;


    /* 
    * Checks if a given page's idle flag is 1'
    * @return true if idle page is 1', false otherwise
    */
    bool is_idle();
};

/* Temporary function to simulate a valid page list
 * @param[in]   page_list           an empty vector of Page type
 * @param[in]   start_addr          starting virtual address to insert to list
 * @param[in]   end_addr            ending virtual address to insert to list
 */
void fill_vector(vector<Page>& page_list, int start_addr, int end_addr);

class Lpet
{
    vector<Page>& page_list;
    int low_thresh;
    int high_thresh;
    bool first_run;
    vector<Page>::iterator start_point;
    
    public:
    
    
    /* Lpet c'tor
    * @param[in]   page_list_ref       a valid page_list
    * @param[in]   high                high threshold in which eviction is needed
    * @param[in]   low                 low threshold in which eviction needs to be stopped
    */
    Lpet(vector<Page>& page_list_ref, int high, int low);


    /* Lpet assignment operator
    * @param[out]  Lpet&               reference to the assigned object, concatenation-compatible
    * @param[in]   a                   the object to which our object would be assigned to
    */
    Lpet& operator=(const Lpet& a);

    /* If eviction is needed - start evicting, else do nothing
        NOTE: the function assumes that page
    
    */
    int run();
};

// class App
// {
//     Lpet lpet;

//     public:
//     vector<Page> page_list;
    
//     App(int high_thresh, int low_thresh);
    
//     int lp_evict();
// };