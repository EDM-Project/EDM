#include "mapTracker.h"
#define INTERVAL 500

bool isSupportedMapping (struct parse_maps* entry) { 
    if (entry->inode != 0) {
        return false;
    }
    std::string pathname = std::string(entry->pathname); 
    if (pathname == "[stack]" || pathname == "[heap]" || pathname == "[vsyscall]" ) {
        return false;
    }
    return true;
}


void MapTracker::updateMaps() { 

    while (true) { 
        struct parse_maps *maps = get_proc_pid_maps(pid);
        struct parse_maps* current = maps;
        while (current != NULL) {
            if (!isSupportedMapping (current)) {
                current = current->next;
                continue;
            }
            if (!monitoredAreas.keyExists(current->start_address, current->end_address)) {
                injectUffdRegister(pid, uffd, current->start_address, current->end_address);
                monitoredAreas.addVMA(VmaMetadata(current));
                uint64_t address = current->start_address;
                while (address <= current->end_address) {
                    if (isPageSwappedOrPresent(pid, address)) { 
                        lspt.Add(Page(address, pid));
                    }
                    address += PAGE_SIZE;
                }
            }
            else { 
            }
            current = current->next;
        }

        free_parse_maps_list(maps);
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL));
    }
    
// while (True)

    // parse proc/maps file
    
    // filter for private anonymous mappings

    // for each new vma
        // inject ptrace_ register vma
        // for each page in vma
            // if present/swapped -> update lspt 

    // sleep interval

}