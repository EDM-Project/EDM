#ifndef PAGE_H
#define PAGE_H
#include <iostream>
#include <stdint.h>
#include "idle_page.h"

class Page{

public:
    Page(uintptr_t vaddr);
    ~Page() = default;
    int page_idle_fd;
    bool is_idle();
    void set_idle();
    uintptr_t vaddr;
    uint64_t pfn;
    void capture();
    char* content;

};

#endif