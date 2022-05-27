#include "EDM_Client.h"
#include <iostream>
EDM_Client::EDM_Client (int argc, char *argv[]){
    std::cout << "EDM_Client constructor" << std::endl;

    this->mpi_instance = new MPI_EDM::MpiApp(argc,argv);
    char* addr = (char*) mmap(NULL, PAGE_SIZE*NUM_OF_PAGES, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    this->ufd = new Userfaultfd(PAGE_SIZE*NUM_OF_PAGES,addr, this->mpi_instance); 
    this->start_addr = addr;
}
EDM_Client::~EDM_Client(){
    Dispose();

}
void EDM_Client::Init(){
    std::cout << "EDM_Client init" << std::endl;

    //init threads
    this->dm_handler_thread = ufd->ActivateUserFaultfd();

}

void EDM_Client::Dispose(){
    //release all
    dm_handler_thread.join();
    lpet_thread.join();

    MPI_Finalize();
}
void EDM_Client::UserThread(){
    std::cout<< "user app started!" <<std::endl;
    //char* addr = (char*) mmap(NULL, PAGE_SIZE*NUM_OF_PAGES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char* addr = this->start_addr;
    for (int i =0 ; i < NUM_OF_PAGES ; i++) {
        char x = addr[i*PAGE_SIZE];
    }
    
    sleep(100);

}

void EDM_Client::RunUserThread(){

    std::thread user_thread (&EDM_Client::UserThread,this);
    user_thread.join();

}
