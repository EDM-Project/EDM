#ifndef SPT_H
#define SPT_H

#include <unordered_map>
#include <utility>      // std::pair, std::make_pair
#include <iostream>
enum page_location { INSTANCE_0, DISK};


class SPT {
private:


public:
std::unordered_map<uintptr_t,page_location> mapping;

SPT() = default;
~SPT() = default;
void AddToSPT(uintptr_t addr, page_location location);
friend std::ostream& operator<<(std::ostream& os, const SPT& spt);

};

std::ostream& operator<<(std::ostream& os, const SPT& spt);

#endif
