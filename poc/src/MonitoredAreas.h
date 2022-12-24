#include <unordered_map>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> /* uint64_t  */
#include "utils/ptrace_do/libptrace_do.h"
#include <mutex>

class MonitoredAreas {
private:
    pid_t pid;
    std::unordered_map<std::pair<uint64_t,uint64_t>,struct parse_maps> areas;
    std::mutex mutex;

public:
    MonitoredAreas(pid_t pid);
    bool Find(uint64_t start_addr,uint64_t end_addr);
    void AddVMA(struct parse_maps entry);
    void RemoveVMA(struct parse_maps entry);

};