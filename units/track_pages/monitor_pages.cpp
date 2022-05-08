#include "idle_page.c"
#include <vector>
#include <iostream>
#include <thread>

#define DEFAULT_INIT 1
#define USEC_EPOCH_MONITORING 5000000



class page_state
{
    int state;

public:

    page_state(int initial_state = DEFAULT_INIT)
    {
        this->state = initial_state;
    }
    void increase()
    {
        if(state >= 3) {return;}
        else {
            state++;
        }
    }
    void decrease()
    {
        if(state <= 0) {return;}
        else{
            state--;
        }
    }
    int get_state()
    {
        return this->state;
    }
};

class lru_entry
{
    uintptr_t vaddr;
    page_state page_status;
public:

    lru_entry() = default;
    ~lru_entry() = default;

    lru_entry(const lru_entry &p1)
    {
        vaddr = p1.vaddr;
        page_status = p1.page_status;
    }

    // This function marks a given virtual address as touched in the last idle flag check
    void page_touch()
    {
        this->page_status.increase();
    }
    uintptr_t get_vaddr()
    {
        return this->vaddr;
    }
    void set_vaddr(uintptr_t vaddr)
    {
        this->vaddr = vaddr;
    }

// This function marks a given virtual address as not touched in the last idle flag check
    void page_detouch()
    {
        this->page_status.decrease();
    }



/*
This function returns the status of a given virtual address:
0 - Not touched recently
1 -
TODO: complete

*/
    int get_status()
    {
        return this->page_status.get_state();
    }
};

std::vector<lru_entry> vec_lru = std::vector<lru_entry>();
uint64_t pfns [PAGES_TO_TEST] = {0};
char* start_vaddr;

void print_pages_states (){
    for(int i=0; i< PAGES_TO_TEST; i++) {
        std::cout<<"---page index " << i << "---" <<std::endl;
        std::cout<<"page vaddr " << vec_lru[i].get_vaddr() <<std::endl;
        std::cout<<"page status " << vec_lru[i].get_status() <<std::endl;
    }
}
void monitor_pages()
{
    while (true) {
        std::cout << "--------NEW EPOCH--------" << std::endl;
        std::cout << "monitor pages: SET IDLE FLAGS" << std::endl;
        set_idle(PAGES_TO_TEST, pfns);
        std::cout <<"monitor pages: SLEEP "<< std::endl;
        usleep(USEC_EPOCH_MONITORING);
        uint8_t results[PAGES_TO_TEST] = {0};
        std::cout <<"monitor pages: GET IDLE returned: ";
        get_idle_flags(PAGES_TO_TEST, pfns, results);
        for (int i=0; i<PAGES_TO_TEST ; i++) {
            std::cout << (int)results[i] << ",";
        }
        std::cout << std::endl;
        for (int i=0; i<PAGES_TO_TEST ; i++) {
            if (results[i] == 0) {
                vec_lru[i].page_touch();
            }else {
                vec_lru[i].page_detouch();
            }
        }
        print_pages_states();
        std::cout <<"_________" << std::endl;
    }
}

using std::vector;

void initialize_test()
{

    start_vaddr = (char*)mmap(NULL, PAGES_TO_TEST * PAGE_SIZE,
                      PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    if(start_vaddr == MAP_FAILED)
    {
        printf("Memory allocation failed!");
        exit(1);
    }

    //make OS allocate frame for each page
    for(int i = 0 ; i < PAGES_TO_TEST ; i++)
    {
        start_vaddr[i*PAGE_SIZE] = 'a';
    }
    //wait for frames getting into lru lists
    sleep(3);

    for(int i = 0 ; i < PAGES_TO_TEST ; i++)
    {
        lru_entry tmp_entry = lru_entry();
        tmp_entry.set_vaddr(reinterpret_cast<uintptr_t>(start_vaddr +(PAGE_SIZE*i)));
        vec_lru.push_back(tmp_entry);
    }
    //for this test- assume pages not going to swap
    for(int i = 0 ; i < PAGES_TO_TEST ; i++)
    {
        uintptr_t vaddr = vec_lru[i].get_vaddr();
        pfns[i] = get_pfn_by_addr(vaddr);
    }
}

void touch_pages(char* mem) {
    int k = 1;
    while(true) {

        for(int i=0; i<PAGES_TO_TEST ; i++) {
            mem[i*PAGE_SIZE] = 'd';
        }
        usleep((USEC_EPOCH_MONITORING/5)*k);
        k++;
    }
}

int main()
{
    initialize_test();
    std::thread monitor_thread(monitor_pages);
    touch_pages(start_vaddr);
    monitor_thread.join();

    return 0;
}