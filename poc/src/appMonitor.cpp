#include "appMonitor.h"
#include <fstream>
#include <iostream>

#define STACK_SIZE 8*1024*1024 

AppMonitor::AppMonitor () { 
    // parse config file (lpet params, app path) -> implement ParseConfigFile()
    
    ParseConfigFile();

    // fork + excev user code

    LOG(DEBUG) << "ParseConfigFile run";

    this->son_pid = RunUserCode();

    LOG(DEBUG) << " RunUserCode run";
    // sigstop (after execv), init uffd of son process, and duplicate fd ()

    //kill(son_pid, SIGSTOP);

    LOG(DEBUG) << " usercode stopped";

    this->redis_instance = new sw::redis::Redis("tcp://127.0.0.1:6380"); 


    this->dm_handler = new DmHandler(this->redis_instance,this,high_threshold,low_threshold, son_pid); 

    LOG(DEBUG) << " DmHandler constructor run";

    // run dmhander(fd) and init redis client

    this->dm_handler_thread = dm_handler->ActivateDM_Handler();

    LOG(DEBUG) << " DmHandler start";


    this->lpet = new Lpet(son_pid, this->redis_instance, lspt, this->high_threshold, this->low_threshold);

    LOG(DEBUG) << " Lpet constructor run";

    // run lpet + proc/maps threads

    this->lpet_thread = std::thread(&AppMonitor::RunLpetThread, this);

    LOG(DEBUG) << " lpet start";

    
    this->map_tracker  = new MapTracker(son_pid, this->dm_handler->getUffdSon(), this->monitored_areas, this->lspt);
    
    LOG(DEBUG) << " MapTracker constructor run";

    this->map_tracker_thread = map_tracker->ActivateMapTracker();

    //this->map_tracker_thread = std::thread(&MapTracker::updateMaps, this->map_tracker);
    //std::thread t (&MapTracker::updateMaps,this);

    LOG(DEBUG) << " MapTracker start";

    // sigcont user code

    //kill(son_pid, SIGCONT);

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
    LOG(DEBUG) << "main function start running";

    AppMonitor a;
    a.dm_handler_thread.join();
    a.lpet_thread.join();
    a.map_tracker_thread.join();
    waitpid(a.getSonPid(), NULL, 0);
}

void AppMonitor::ParseConfigFile () {
    std::ifstream ReadFile("config.txt");
	if(!ReadFile.is_open()) {
		LOG(ERROR) << "Error: Could not open file.\n";
	}
	std::string word;
	while(ReadFile >> word) {
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


// AppMonitor::AppMonitor () 
//  {

//     LOG(DEBUG) << "[EDM CLIENT] -  INIT";
//     ParseConfigFile();
//     setenv("start_addr",std::to_string(start_addr).c_str(),1);
//     setenv("end_addr",std::to_string(end_addr).c_str(),1);
//     this->redis_instance = new sw::redis::Redis("tcp://127.0.0.1:6380"); 
//     /* REDIS_INTEGRATION: for phase I we need one redis server as localhost.
//     tcp is optional. advanced options can be set using the ConnectionOptions data structure later
//     port number could be changed according to the intended server's open port*/
//     this->ufd = new DmHandler(this->redis_instance,this,high_threshold,low_threshold); 
//     this->dm_handler_thread = ufd->ActivateDM_Handler();
//     this->lpet = new Lpet(this->redis_instance, lspt, this->high_threshold, this->low_threshold);
//     this->lpet_thread = std::thread(&AppMonitor::RunLpetThread, this);
// }   



// AppMonitor::~AppMonitor(){
//     //release all
//     LOG(DEBUG)<< "[EDM CLIENT] - SHUTDOWN!";
//     dm_handler_thread.join();
//     lpet_thread.join();

//     delete ufd;
//     delete redis_instance;
// }


void AppMonitor::WaitForRunLpet() {
    std::unique_lock<std::mutex> lck(run_lpet_mutex);
    while(lspt.GetSize() < high_threshold) cv.wait(lck);


}
void AppMonitor::RunLpetThread() {
    while (true) {
        is_lpet_running = false;
        WaitForRunLpet();
        //now we know that page list has max 
        LOG(DEBUG) << "[CLIENT] - reached high threshold, running lpet";
        is_lpet_running = true;
        std::thread lpet_thread = lpet->ActivateLpet();
        cv.notify_all();
        lpet_thread.join();
        LOG(DEBUG) << "[CLIENT] - lpet end running .";
        is_lpet_running = false;
        cv.notify_all();
    }
}

