#include "SPT.h"
#include <utility>      // std::pair, std::make_pair

void SPT::UpdateSPT(uintptr_t addr, page_location location){
    mapping[addr] = location;
}
bool SPT::IsAddrExist(uintptr_t addr){
    if(mapping.find(addr) != mapping.end() ){
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const SPT& spt){
    os << "DMS- Pring SPT STATE" <<std::endl;
    os << "-----START SPT ----" << std::endl;
    for (auto& it: spt.mapping) {
        // Do stuff
        os <<  "address : " << it.first; 
        os << "  ";
        switch (it.second)
        {
        case INSTANCE_0:
            os << "location : " << "INSTANCE_0";
            break;
        case DISK:
            os << "location : " << "DISK";
        default:
            break;
        }
        os << std::endl;
    }
    os << "------END SPT-----" <<std::endl;
}