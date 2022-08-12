/*
#include "mpi.h"
#include "../shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "EDM_Client.h"
*/
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096 

int main(int argc, char *argv[])
{   
   std::cout << "User code main function start running" << std::endl;


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

   return 0;
}