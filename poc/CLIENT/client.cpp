#include "client.h"
#include <fstream>
#include <iostream>
Client::Client () 
 {

    LOG(DEBUG) << "[EDM CLIENT] -  INIT";
    ParseConfigFile();
    setenv("start_addr",std::to_string(start_addr).c_str(),1);
    setenv("end_addr",std::to_string(end_addr).c_str(),1);
    this->mpi_instance = new MPI_EDM::MpiClient();
    this->ufd = new DmHandler(this->mpi_instance,this,high_threshold,low_threshold); 
    this->dm_handler_thread = ufd->ActivateDM_Handler();
    this->lpet = new Lpet(this->mpi_instance, lspt, this->high_threshold, this->low_threshold);
    this->lpet_thread = std::thread(&Client::RunLpetThread, this);
}

void Client::ParseConfigFile () {
    std::ifstream ReadFile("CLIENT/client_config.txt");
	if(!ReadFile.is_open()) {
		LOG(ERROR) << "Error: Could not open file.\n";
	}
	std::string word;
	while(ReadFile >> word) {
		if(word == "start_addr") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->start_addr  = std::stoi (word.c_str(),nullptr,16);
		}
		else if(word == "end_addr") {
			ReadFile.ignore(3);
			ReadFile >> word;
            this->end_addr  = std::stoi (word.c_str(),nullptr,16);
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


Client::~Client(){
    //release all
    LOG(DEBUG)<< "[EDM CLIENT] - SHUTDOWN!";
    dm_handler_thread.join();
    lpet_thread.join();

    MPI_Finalize();
    delete ufd;
    delete mpi_instance;
}


void Client::WaitForRunLpet() {
    std::unique_lock<std::mutex> lck(run_lpet_mutex);
    while(lspt.GetSize() < high_threshold) cv.wait(lck);

}
void Client::RunLpetThread() {
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

Client client;
