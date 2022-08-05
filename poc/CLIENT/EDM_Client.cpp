#include "EDM_Client.h"
#include <iostream>
EDM_Client::EDM_Client (int argc, char *argv[]){

    start_addr = std::stoi (argv[1],nullptr,16);
    end_addr  = std::stoi (argv[1],nullptr,16);
    low_threshold = std::atoi(argv[3]);
    high_threshold = std::atoi(argv[4]);

    this->mpi_instance = new MPI_EDM::MpiApp(argc,argv);
    
    this->ufd = new Userfaultfd(this->mpi_instance,this); 
    this->dm_handler_thread = ufd->ActivateDM_Handler();
}
EDM_Client::~EDM_Client(){
    //release all
    std::cout<< "EDM CLIENT SHUTDOWN!" <<std::endl;
    dm_handler_thread.join();
    lpet_thread.join();

    MPI_Finalize();
    delete ufd;
    delete mpi_instance;
}

void EDM_Client::AddToPageList(uintptr_t addr) {
    pages_list.push_back(addr);
    /*
    std::cout << "CURRENT PAGE LIST:" <<std::endl;
    for (auto i :pages_list){
        std::cout << i << ", ";
    }
    */
}


void EDM_Client::UserThread(){
    
    
    char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    
    char* area_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    char* area_3 = (char*) mmap( (void*)0x1E78000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);                       

    area_1[0] = 'x';
    std::cout<< "usercode : area_1[0] " << area_1[0] << std::endl;
    area_2[0] = 'y';
    std::cout<< "usercode : area_2[0] " << area_2[0] << std::endl;
    area_3[0] = 'z';
    std::cout<< "usercode : area_3[0] " << area_3[0] << std::endl;
 

    return;
}

void EDM_Client::RunUserThread(){

    std::thread user_thread (&EDM_Client::UserThread,this);
    user_thread.join();

}
