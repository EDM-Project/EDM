#include "Page.h"
// #include "idle_page.c"

Page::Page(uintptr_t vaddr) : vaddr(vaddr) ,pfn(get_pfn_by_addr(vaddr)) 
{
    content = new char[PAGE_SIZE];
    // usleep(500000);
    // this->set_idle();
};

bool Page::is_idle()
{
    uint8_t retval;
    get_idle_flags(1, &this->pfn, &retval);
    return retval;
}

void Page::set_idle()
{
    set_idle_pages(1, &this->pfn);
}

void Page::capture()
{
    for(int i = 0 ; i < PAGE_SIZE ; i++)
    {
        content[i] = char((this->vaddr) + i);
    }
}