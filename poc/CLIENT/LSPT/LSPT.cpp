#include "LSPT.h"
#include <utility>      // std::pair, std::make_pair

void LSPT::AddPage(uintptr_t vaddr) {
    pages_list.push_back(Page(vaddr));
}


std::ostream& operator<<(std::ostream& os, LSPT& lspt){
    os << "Client- Pring LSPT STATE" <<std::endl;
    os << "-----START LSPT ----" << std::endl;
    for (auto& it: lspt.pages_list) {
        
        os << "address : " << PRINT_AS_HEX(it.vaddr) << std::dec; 
        os << "  ";
        os << "pfn : " << it.pfn;
        os << "  ";
        os << it.is_idle();
        os << std::endl;
    }
    os << "------END SPT-----" <<std::endl;
}