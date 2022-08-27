#include "client.h"
#include <fstream>
#include <iostream>
Client::Client () 
 {

    LOG(DEBUG) << "[EDM CLIENT] -  INIT";
    ParseConfigFile();
    setenv("start_addr",std::to_string(start_addr).c_str(),1);
    setenv("end_addr",std::to_string(end_addr).c_str(),1);
    this->mpi_instance = new MPI_EDM::MpiClient(0,NULL);
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
        LOG(DEBUG) << "[CLIENT] - reached high threshold. run lpet cycle";
        is_lpet_running = true;
        std::thread lpet_thread = lpet->ActivateLpet();
        LOG(DEBUG) << "[CLIENT] - lpet trigged";
        cv.notify_all();
        lpet_thread.join();
        LOG(DEBUG) << "[CLIENT] - lpet end running cycle.";
        is_lpet_running = false;
        cv.notify_all();
    }
    //return lpet->run();
}


void Client::UserThread(){
    
    
    char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    
    char* area_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    char* area_3 = (char*) mmap( (void*)0x1E78000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);                       

    area_1[0] = 'x';
    LOG(DEBUG)<< "usercode : area_1[0] " << area_1[0];
    area_2[0] = 'y';
    LOG(DEBUG)<< "usercode : area_2[0] " << area_2[0];
    area_3[0] = 'z';
    LOG(DEBUG)<< "usercode : area_3[0] " << area_3[0];
 

    return;
}

void Client::RunUserThread(){

    std::thread user_thread (&Client::UserThread,this);
    user_thread.join();

}
Client client;
