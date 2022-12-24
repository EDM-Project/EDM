#include "appMonitor.h"
#include <fstream>
#include <iostream>

AppMonitor::AppMonitor () { 
    // parse config file (lpet params, app path) -> implement ParseConfigFile()
    
    // fork + excev user code

    // sigstop (after execv), init uffd of son process, and duplicate fd ()

    // run dmhander(fd) and init redis client

    // run lpet + proc/maps threads

    // sigcont user code

    
}


AppMonitor::~AppMonitor() {

   // 

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

// void AppMonitor::ParseConfigFile () {
//     std::ifstream ReadFile("CLIENT/client_config.txt");
// 	if(!ReadFile.is_open()) {
// 		LOG(ERROR) << "Error: Could not open file.\n";
// 	}
// 	std::string word;
// 	while(ReadFile >> word) {
// 		if(word == "start_addr") {
// 			ReadFile.ignore(3);
// 			ReadFile >> word;
//             this->start_addr  = std::stoi (word.c_str(),nullptr,16);
// 		}
// 		else if(word == "end_addr") {
// 			ReadFile.ignore(3);
// 			ReadFile >> word;
//             this->end_addr  = std::stoi (word.c_str(),nullptr,16);
// 		}
// 		else if(word == "low_threshold") {
// 			ReadFile.ignore(3);
// 			ReadFile >> word;
//             this->low_threshold =  std::atoi(word.c_str());
// 		}
// 		else if(word == "high_threshold") {
// 			ReadFile.ignore(3);
// 			ReadFile >> word;
//             this->high_threshold =  std::atoi(word.c_str());
// 		}
// 	}
// 	ReadFile.close();

// }


// AppMonitor::~AppMonitor(){
//     //release all
//     LOG(DEBUG)<< "[EDM CLIENT] - SHUTDOWN!";
//     dm_handler_thread.join();
//     lpet_thread.join();

//     delete ufd;
//     delete redis_instance;
// }


// void AppMonitor::WaitForRunLpet() {
//     std::unique_lock<std::mutex> lck(run_lpet_mutex);
//     while(lspt.GetSize() < high_threshold) cv.wait(lck);


// }
// void AppMonitor::RunLpetThread() {
//     while (true) {
//         is_lpet_running = false;
//         WaitForRunLpet();
//         //now we know that page list has max 
//         LOG(DEBUG) << "[CLIENT] - reached high threshold, running lpet";
//         is_lpet_running = true;
//         std::thread lpet_thread = lpet->ActivateLpet();
//         cv.notify_all();
//         lpet_thread.join();
//         LOG(DEBUG) << "[CLIENT] - lpet end running .";
//         is_lpet_running = false;
//         cv.notify_all();
//     }
// }

// AppMonitor client;
