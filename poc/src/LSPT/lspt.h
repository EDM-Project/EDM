#ifndef LSPT_H
#define LSPT_H

#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


#include <vector>
#include <algorithm>
#include <utility>      // std::pair, std::make_pair
#include <iostream>
#include <string>

#include "../utils/logger.h"
#include "page.h"

class LSPT {
private:
std::vector<Page> page_list;
std::mutex mutex;

public:

LSPT() = default;
~LSPT() = default;
void Add(Page page);
Page AtIndex(int index);
void Remove(int index);
int GetSize();
bool IsPageExist(uintptr_t vaddr);

friend std::ostream& operator<<(std::ostream& os, const LSPT& lspt);

};

std::ostream& operator<<(std::ostream& os, const LSPT& lspt);

#endif
