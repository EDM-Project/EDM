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

    if (entry->start_address > 0x1D56000 ) {
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
    
// while (True)

    // parse proc/maps file
    
    // filter for private anonymous mappings

    // for each new vma
        // inject ptrace_ register vma
        // for each page in vma
            // if present/swapped -> update lspt 

    // sleep interval

}


void staticUpdateMaps(pid_t pid,long uffd, MonitoredAreas* monitoredAreas, LSPT* lspt) { 

    printf("--------->Inside updateMaps CPP  static thread\n");
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
            if (!monitoredAreas->keyExists(current->start_address, current->end_address)) {
                LOG(DEBUG) << "[MapTracker] add vma:" << current->start_address;
                injectUffdRegister(pid, uffd, current->start_address, current->end_address);
                monitoredAreas->addVMA(VmaMetadata(current));
                uint64_t address = current->start_address;
                while (address <= current->end_address) {
                    if (isPageSwappedOrPresent(pid, address)) { 
                        lspt->Add(Page(address, pid));
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


void* func(void* arg)
{
    printf("--------->Inside the thread\n");
    pid_t pid =  *((pid_t*)arg);
    printf("pid is : %d\n",pid);
    injectMmap(pid,256);
    printf("--------->Inside the thread\n");
    injectMmap(pid,256);
    printf("--------->Inside the thread\n");
    injectMmap(pid,256);
    printf("--------->Inside the thread\n");
    injectMmap(pid,256);
    printf("--------->Inside the thread\n");
    // exit the current thread

    pthread_exit(NULL);
}
void foo (pid_t pid) { 
    printf("--------->Inside CPP thread\n");
    injectMmap(pid,512);
}

std::thread MapTracker::ActivateMapTracker(){

/*    
    pthread_t ptid;
  
    pthread_create(&ptid, NULL, &func, &pid);
    pthread_join(ptid, NULL);

    

    std::thread t (&MapTracker::updateMaps,this);

    return t;
    */
   std::thread e(foo, pid);
    e.join();
        std::thread q(std::bind(staticUpdateMaps,pid,uffd, &monitoredAreas,&lspt));
        q.join();
        return q;

}

