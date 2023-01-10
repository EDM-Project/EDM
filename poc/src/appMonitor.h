#ifndef APPMONITOR_H
#define APPMONITOR_H

#include <thread>
#include <vector>
#include <condition_variable> 
#include <algorithm>
#include <signal.h>
#include "utils/logger.h"
#include "LSPT/lpet.h"
#include "LSPT/page.h"
#include "LSPT/lspt.h"
#include "monitoredAreas.h"
#include "mapTracker.h"
#include "DmHandler/dmHandler.h"
#include <sw/redis++/redis++.h>
#include "redisClient.h"

class AppMonitor {

private:
    
    std::string binary_path;
    std::string redis_uri;
    int high_threshold;
    int low_threshold;
    
    char* child_stack;
    pid_t son_pid;

    Lpet* lpet;    
    DmHandler* dm_handler;
    MapTracker* map_tracker;

    
    void ParseConfigFile ();

public:
    std::mutex run_lpet_mutex;
    std::condition_variable cv;
    
    std::thread dm_handler_thread;
    std::thread lpet_thread;
    std::thread map_tracker_thread;

    LSPT lspt;    
    MonitoredAreas monitored_areas;

    //for debug
    bool is_lpet_running;

    AppMonitor();
    ~AppMonitor();

    static int child_function(void *arg);
    pid_t RunUserCode();
    void RunLpetThread();
    void WaitForRunLpet();
    pid_t getSonPid() { 
        return son_pid;
    }

    static void replaceRedisClient(int signum);

};
#endif