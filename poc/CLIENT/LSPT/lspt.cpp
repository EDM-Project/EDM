#include "lspt.h"


void LSPT::Add(Page page) { 
    std::lock_guard<std::mutex> lockGuard(mutex);
    page_list.push_back(page);

}
void LSPT::Remove(int index) {
    std::lock_guard<std::mutex> lockGuard(mutex);
    page_list.erase(page_list.begin() + index);

}
int LSPT::GetSize() {
    std::lock_guard<std::mutex> lockGuard(mutex);
    LOG(DEBUG) << "[LSPT] - lspt size is: " << page_list.size();
    return page_list.size();

} 
bool LSPT::IsPageExist(uintptr_t vaddr) { 
    for (auto& page : page_list){
        if (page.vaddr == vaddr){
            return true;
        }
    }
    return false;

}
Page LSPT::AtIndex(int index) { 
    return page_list[index];
}

std::ostream& operator<<(std::ostream& os, const LSPT& lspt){
    os << "page list state: " <<std::endl;
    os << "-----START LSPT ----" << std::endl;
    for (auto& it: lspt.page_list) {
        os << it;
    }
    os << "-----END LSPT ----" << std::endl;
    os << std::endl;
    return os;
}