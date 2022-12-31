#ifndef MONITORED_AREASH
#define MONITORED_AREASH

#include <unordered_map>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> /* uint64_t  */
#include <mutex>
#include "utils/syscallInjectors.h"
#include "utils/helperfunc.h"
#include "vmaMetadata.h"
extern "C" { 
    #include "utils/ptrace_do/libptrace_do.h"
}


typedef std::pair<uint64_t, uint64_t> address_pair;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

class MonitoredAreas {
private:
    std::unordered_map<address_pair,VmaMetadata,pair_hash> areas;
    std::mutex mutex;

public:
    MonitoredAreas() = default;
    bool keyExists(uint64_t start_addr,uint64_t end_addr);
    void addVMA(VmaMetadata entry);
    void removeVMA(VmaMetadata entry);

};
#endif