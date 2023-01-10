#include "appMonitor.h"
#include <fstream>
#include <iostream>

#define STACK_SIZE 8*1024*1024 

AppMonitor::AppMonitor () { 

    // parse config file (lpet params, app path) -> implement ParseConfigFile()
    
    ParseConfigFile();

    // fork + excev user code

    this->son_pid = RunUserCode();

    LOG(DEBUG) << " RunUserCode run";
    // sigstop (after execv), init uffd of son process, and duplicate fd ()

    kill(son_pid, SIGSTOP);

    LOG(DEBUG) << " usercode stopped";

    this->redis_instance = new sw::redis::Redis(this->redis_uri); 


    this->dm_handler = new DmHandler(this->redis_instance,this,high_threshold,low_threshold, son_pid); 

    // run dmhander(fd) and init redis client

    this->dm_handler_thread = dm_handler->ActivateDM_Handler();

    LOG(DEBUG) << " DmHandler start";


    this->lpet = new Lpet(son_pid, this->redis_instance, lspt, this->high_threshold, this->low_threshold);

    // run lpet + proc/maps threads

    this->lpet_thread = std::thread(&AppMonitor::RunLpetThread, this);

    LOG(DEBUG) << " lpet start";

    
    this->map_tracker  = new MapTracker(son_pid, this->dm_handler->getUffdSon(), this->monitored_areas, this->lspt);
    
    this->map_tracker_thread = map_tracker->ActivateMapTracker();

    LOG(DEBUG) << " MapTracker start";

    // sigcont user code

    kill(son_pid, SIGCONT);

    LOG(DEBUG) << " user code resume";


}

int AppMonitor::child_function(void *arg) {
    // Do some work here
    char* path =  (char*)arg;
    execve(path, NULL, NULL);

    return 0;
}

pid_t AppMonitor::RunUserCode() { 


    this->child_stack = (char*) malloc(STACK_SIZE*sizeof(char));

    pid_t child_pid;
    auto path = this->binary_path.c_str();
    child_pid = clone(AppMonitor::child_function, child_stack + STACK_SIZE, CLONE_VFORK | SIGCHLD,  (void*) path );
    if (child_pid == -1) {
        LOG(ERROR) << "Failed to clone user code"; 
    }
    return child_pid;

}




AppMonitor::~AppMonitor() {

//    dm_handler_thread.join();
//    lpet_thread.join();
//    map_tracker_thread.join();
   free(child_stack);

}

int main() { 

    LOG(DEBUG) << "main function start running. pid: " << getpid();
    //signal(SIGUSR1, AppMonitor::ReplaceRedisClient);

    AppMonitor a;
    a.dm_handler_thread.join();
    a.lpet_thread.join();
    a.map_tracker_thread.join();
    int status;
    waitpid(a.getSonPid(), &status, 0);
   
}

void AppMonitor::ParseConfigFile () {
    std::ifstream ReadFile("config.txt");
	if(!ReadFile.is_open()) {
		LOG(ERROR) << "Error: Could not open file.\n";
	}
	std::string word;
	while(ReadFile >> word) {
        if (word == "redis_uri") { 
            ReadFile.ignore(3);
			ReadFile >> word;
            this->redis_uri  = std::string(word);
        }
		if(word == "binary_path") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->binary_path  = std::string(word);
		}	
		else if(word == "low_threshold") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->low_threshold =  std::atoi(word.c_str());
		}
		else if(word == "high_threshold") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->high_threshold =  std::atoi(word.c_str());
		}
	}
	ReadFile.close();

}


void AppMonitor::WaitForRunLpet() {
    std::unique_lock<std::mutex> lck(run_lpet_mutex);
    while(lspt.GetSize() < high_threshold) cv.wait(lck);


}
void AppMonitor::RunLpetThread() {
    while (true) {
        is_lpet_running = false;
        WaitForRunLpet();
        //now we know that page list has max 
        LOG(DEBUG) << "[AppMonitor] - reached high threshold, running lpet";
        is_lpet_running = true;
        std::thread lpet_thread = lpet->ActivateLpet();
        cv.notify_all();
        lpet_thread.join();
        LOG(DEBUG) << "[AppMonitor] - lpet end running .";
        is_lpet_running = false;
        cv.notify_all();
    }
}

void AppMonitor::ReplaceRedisClient() { 
    this->ParseConfigFile();
    this->redis_instance = new sw::redis::Redis(this->redis_uri); 
}

