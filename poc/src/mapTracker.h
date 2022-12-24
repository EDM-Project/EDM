#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> /* uint64_t  */
#include "LSPT/lspt.h"
class MapTracker { 

private: 
    pid_t pid;
    LSPT& lspt;

public:
    MapTracker(pid_t pid,LSPT& lspt) : pid(pid),lspt(lspt) {}
    void UpdateMaps();
    ~MapTracker() = default;

};