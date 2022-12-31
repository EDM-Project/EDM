#ifndef APPMONITOR_H
#define APPMONITOR_H

#include <thread>
#include <vector>
#include <condition_variable> 
#include <algorithm>
#include "utils/logger.h"
#include "LSPT/lpet.h"
#include "LSPT/page.h"
#include "LSPT/lspt.h"
#include "monitoredAreas.h"
#include "DmHandler/dmHandler.h"
#include <sw/redis++/redis++.h>


class AppMonitor {

private:
    
    std::string path;
    int high_threshold;
    int low_threshold;
    
    
    Lpet* lpet;    
    DmHandler* ufd;

    sw::redis::Redis* redis_instance;
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    
    void ParseConfigFile ();

public:
    std::mutex run_lpet_mutex;
    std::condition_variable cv;
    
    LSPT lspt;    
    MonitoredAreas monitoredAreas;

    //for debug
    bool is_lpet_running;

    AppMonitor ();
    ~AppMonitor();
    void RunLpetThread();
    void WaitForRunLpet();
    
    

};
#endif