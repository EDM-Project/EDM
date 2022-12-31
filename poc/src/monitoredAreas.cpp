#include "monitoredAreas.h"

bool MonitoredAreas::keyExists(uint64_t start_addr,uint64_t end_addr){ 
    std::lock_guard<std::mutex> lock(mutex);
    if  (areas.find(std::make_pair(start_addr,end_addr)) != areas.end()) { 
        return true;
    }
    return false;        
}

void MonitoredAreas::addVMA(VmaMetadata entry) { 
    std::lock_guard<std::mutex> lock(mutex);
    LOG(DEBUG) << "vma: start: " << convertToHexRep(entry.start_address) << " end: " << convertToHexRep(entry.end_address) << " added to MonitoredAreas";
    areas.insert({std::make_pair(entry.start_address, entry.end_address),entry});
}

void MonitoredAreas::removeVMA(VmaMetadata entry) { 
    std::lock_guard<std::mutex> lock(mutex);
    auto key =std::make_pair(entry.start_address, entry.end_address);
    areas.erase(key);
}

