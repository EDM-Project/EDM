#include "EDM_Client.h"
#include <iostream>
EDM_Client::EDM_Client (int argc, char *argv[]){

    start_addr = std::stoi (argv[1],nullptr,16);
    num_of_pages = std::atoi(argv[2]);

    this->mpi_instance = new MPI_EDM::MpiApp(argc,argv);
    char* addr = (char*) mmap((void*)start_addr, PAGE_SIZE*num_of_pages, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (addr != (char*)start_addr) {
        std::cout<< "mmap failed!" << std::endl;
        exit(1);
    }
    this->ufd = new Userfaultfd(PAGE_SIZE*num_of_pages,addr, this->mpi_instance,this); 
    this->dm_handler_thread = ufd->ActivateDM_Handler();
}
EDM_Client::~EDM_Client(){
    //release all
    std::cout<< "EDM CLIENT SHUTDOWN!" <<std::endl;
    munmap((char*)start_addr, num_of_pages*PAGE_SIZE);
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
    
    /*
    char* addr = (char*) mmap((void*)start_addr, PAGE_SIZE*2, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (addr != (char*)start_addr) {
        std::cout<< "mmap failed!" << std::endl;
        exit(1);
    }
    addr[0] = 'x';
    sleep(4);
    std::cout << "endddddd" << std::endl;
    */
    
    char* addr = (char*)this->start_addr;
    for (int i =0 ; i < num_of_pages ; i++) {
        std::cout << "User App - touch page in addr: " << (start_addr + i*PAGE_SIZE) << std::endl;
        char x = addr[i*PAGE_SIZE];
    }
    sleep(1);
    char* new_one = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    
    new_one[0] = 'x';

    return;
}

void EDM_Client::RunUserThread(){

    std::thread user_thread (&EDM_Client::UserThread,this);
    user_thread.join();

}
