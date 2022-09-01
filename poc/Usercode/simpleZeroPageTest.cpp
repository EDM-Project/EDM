#include "tests.h"

#define PAGE_SIZE 4096

void simpleZeroPageTest() {

   LOG(DEBUG) << "[Usercode] : User code main function start running" ;

   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
    /* 0x1D4C000 is in shared virtual space, so the memory mapping is registered in userfaultfd */
    area_1[0] = 'x';

    /*
      accessing area_1 first byte, causes page fault, eventually, zerepage will be allocated.
        memory layout diagram:

      +--------+     high address
      |        |  
      | stack  |  |
      |        |  |
      |        |  v
      +--------+  
      | .....  |
      +--------+  
      | area_1 |  <-0x1D4C000
      +--------+  
      | .....  |
      +--------+  
      |        |  ^
      |        |  |
      | heap   |  |
      |        |  
      +--------+  
      |        |  
      | .bss   |
      +--------+
      |        |  
      | .data  |
      +--------+
      |        |   low address
      | .text  |
      +--------+



    */      

   LOG(DEBUG)<< "[Usercode] : area_1[0] " << area_1[0] ;
   

}