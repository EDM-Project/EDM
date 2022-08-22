#ifndef LSPT_H
#define LSPT_H

#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


#include <vector>
#include <utility>      // std::pair, std::make_pair
#include <iostream>
#include <string>
#include "Page.h"

class LSPT {
private:

public:
std::vector<Page> pages_list;



LSPT() = default;
~LSPT() = default;

void AddPage(uintptr_t vaddr);
friend std::ostream& operator<<(std::ostream& os, LSPT& lspt);

};

std::ostream& operator<<(std::ostream& os, const LSPT& spt);

#endif