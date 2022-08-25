#ifndef SPT_H
#define SPT_H

#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <utility>      // std::pair, std::make_pair
#include <iostream>
#include <string>

#include "../shared/logger.h"
enum page_location { INSTANCE_0, DISK};


class SPT {
private:
std::unordered_map<uintptr_t,page_location> mapping;


public:

SPT() = default;
~SPT() = default;
void UpdateSPT(uintptr_t addr, page_location location);
bool IsAddrExist(uintptr_t addr);
friend std::ostream& operator<<(std::ostream& os, const SPT& spt);

};

std::ostream& operator<<(std::ostream& os, const SPT& spt);

#endif
