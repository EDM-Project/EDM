#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> /* uint64_t  */
#include <chrono>
#include <thread>
#include <functional>
#include "LSPT/lspt.h"
#include "monitoredAreas.h"
#include "utils/syscallInjectors.h"
// extern "C" {
//     #include "utils/ptrace_do/libptrace_do.h"
// }

class MapTracker { 

private: 
    pid_t pid;
    int uffd;
    MonitoredAreas& monitoredAreas;
    LSPT& lspt;

public:
    MapTracker(pid_t pid, int uffd, MonitoredAreas& monitoredAreas,LSPT& lspt) : pid(pid), uffd(uffd), monitoredAreas(monitoredAreas), lspt(lspt) {}
    std::thread ActivateMapTracker();
    void updateMaps();
    ~MapTracker() = default;

};