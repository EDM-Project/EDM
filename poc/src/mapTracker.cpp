#include "mapTracker.h"
#define INTERVAL 500

bool isSupportedMapping (struct parse_maps* entry) { 
    if (entry->inode != 0) {
        return false;
    }
    std::string pathname = std::string(entry->pathname); 
    if (pathname == "[stack]" || pathname == "[heap]" || pathname == "[vsyscall]" || pathname == "[vvar]" || pathname == "[vdso]") {
        return false;
    }

    return true;
}


void MapTracker::updateMaps() { 

    printf("--------->Inside updateMaps CPP thread\n");
    injectMmap(pid,512);

    LOG(DEBUG) << "[MapTracker] - start parsing maps: pid:" << pid;
    int i = 0; 
    while (true) { 
        LOG(DEBUG) << "[MapTracker] start iteration num: " << i;
        i++;
        struct parse_maps *maps = get_proc_pid_maps(pid);

        struct parse_maps* current = maps;
        while (current) {
            if (!isSupportedMapping (current)) {
                current = current->next;
                continue;
            }
            if (!monitoredAreas.keyExists(current->start_address, current->end_address)) {
                LOG(DEBUG) << "[MapTracker] add vma:" << current->start_address;
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

}

std::thread MapTracker::ActivateMapTracker(){
   
    std::thread t (&MapTracker::updateMaps,this);
    return t;

}

