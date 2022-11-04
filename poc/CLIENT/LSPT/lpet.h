#ifndef LPET_H
#define LPET_H
#include <vector>
#include "page.h"
#include "../../shared/logger.h"
#include "lspt.h"
#include "../mpiClient/mpiClient.h"
#include <sw/redis++/redis++.h>
#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec


using std::vector;

class Lpet{

    bool first_run;
    //vector<Page>::iterator start_point;
    LSPT& lspt;
    int start_point;
    uint32_t low_thresh;
    uint32_t high_thresh;
    sw::redis::Redis redis_instance;
    char* buffer;



public:

    Lpet(sw::redis::Redis redis_instance, LSPT& lspt, int high, int low);
    Lpet() = default;
    ~Lpet() = default;
    Lpet& operator=(const Lpet& a);
    uint32_t run();
    std::thread ActivateLpet();
};



#endif