/*
#include "mpi.h"
#include "../shared/MpiEdm.h"
#include "userfaultfd/userfaultfd.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "EDM_Client.h"
*/
#include "../shared/CPPLogger.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>


#define PAGE_SIZE 4096 
#define MREMAP_DONTUNMAP	4

void test_simple_flow_eviction() {

   //area_1 - start in 0x1D4C000 
   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE *10 , PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *10 ; i+= PAGE_SIZE ) {
      area_1[i] = 'x';
   }
   
   char* area_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE *10 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   for (int i =0; i < PAGE_SIZE *10 ; i+= PAGE_SIZE ) {
      area_2[i] = 'y';
   }
   /*
   NOW, the memory layout should look like this.
   20 pages allocated which means we is the maximum allowed. 
   .__________.
   . 0x1D4C000
   .          .
   .          .
   . 0x1D56000.
   .__________.
   .
   .__________.
   .0x1E14000 .
   .          .
   .          .
   .0X1E1E000 .
   .__________.

   */
   char* area_3 = (char*) mmap( (void*)0x1E82000, PAGE_SIZE *1 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);   
   area_3[0] = 'z';   
   // this page fault should evict all the first 100 pages
   // so the memory layout should looks like this: 
   /*
   .__________.
   .0x1D4C000 .
   .   EMPTY  .
   .   DATA   .
   .0x1D56000 .
   .__________.
   .
   .__________.
   .0x1E14000 .
   .          .
   .          .
   .0X1E1E000 .
   .__________.
   .          .
   .__________.
   .0x1E82000 . 
   .__________.

   */

   char temp = area_1[0];

   // this line should trigged page fault, which will be solved by getting the page from dms
   LOG(DEBUG) << "first char of area_1 is :" << (char)temp;
   printf("first char of area_1 is %c\n", temp);
}

void test_mremap() {

   LOG(DEBUG) << "[Usercode] : User code test_mremap function start running" ;

   char* addr_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   char* addr_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);            
   addr_1[0] = 'x';
   addr_2[0] = 'y';
   mremap(addr_1,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, addr_1 - 40960 );
   mremap(addr_2,PAGE_SIZE,PAGE_SIZE,MREMAP_MAYMOVE | MREMAP_DONTUNMAP | MREMAP_FIXED, addr_1 - 40960 );
   addr_1[0] = 'z';
   addr_2[0] = 'w';

}



int main(int argc, char *argv[])
{ 
     test_simple_flow_eviction();

/*

   LOG(DEBUG) << "[Usercode] : User code main function start running" ;


   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    
   char* area_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   char* area_3 = (char*) mmap( (void*)0x1E78000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);                       

   area_1[0] = 'x';
   LOG(DEBUG)<< "[Usercode] : area_1[0] " << area_1[0] ;
   area_2[0] = 'y';
   LOG(DEBUG)<< "[Usercode] : area_2[0] " << area_2[0] ;
   area_3[0] = 'z';
   LOG(DEBUG)<< "[Usercode] : area_3[0] " << area_3[0] ;
*/
   return 0;
}