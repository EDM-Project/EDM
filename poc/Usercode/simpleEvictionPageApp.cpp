#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include "logger.h"

#define PAGE_SIZE 4096

void waitFor(int time_in_seconds) { 
  int cnt = 0; 

  while (cnt < time_in_seconds)
  {
      time_t current_time;
      time(&current_time);
     // printf("PID: %d, Current time: %s", getpid(), ctime(&current_time));

      sleep(1);
      cnt++;
  }
}

static bool comparePages(char* source, char* dest){ 
   if (memcmp(source,dest,PAGE_SIZE) == 0) {
      return true; 
   }
   return false;
}

/**
 * @brief simple test to show eviction of page to disc, and getting this page back.
 *      configuration: high_threshold = 4 , low_threshold = 2
 */
int main() {
    LOG(INFO) << "[Usercode] : User code main function start running";
    waitFor(2);
    LOG(INFO) <<  "[Usercode] : mmap vma, start address: 0x1D4C000 , size : 4 pages ";
    char* area_1 = (char*) mmap( (void*)0x1D4C000, 4 *PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0); 
    waitFor(3); 
    for (int i=0 ; i < 4 *PAGE_SIZE ; i++) {
        area_1[i] = 'x';
    }
    /* save area_1 first page in buffer for validation */
    
    char* area_1_first_page = (char*)malloc(PAGE_SIZE);
    memcpy(area_1_first_page,area_1,PAGE_SIZE);

    /*
     memory layout diagram:
      
          +--------+     high address
          |        |  
          | stack  |  |
          |        |  |
          |        |  v
          +--------+  
          | .....  |
          +--------+                  
      |   | page 4 |               
      |   +--------+                
      |   | page 3 |              
area_1|   +--------+                
      |   | page 2 |                 
      |   +--------+                 
      |   | page 1 |  <-0x1D4C000  
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
    LOG(INFO) <<  "[Usercode] : mmap vma, start address: 0x1D5C000 , size : 1 pages ";
    char* area_2 = (char*) mmap( (void*)0x1D5C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
    waitFor(2);
    area_2[0] = 'z';

    /*

      accessing area_1 first byte, shoud trigger lpet, which will evict the first two pages
        memory layout diagram:

   
     memory layout diagram:
     
        
          +--------+  
          | area_2 |  <-0x1D5C000
          +--------+  
          | .....  |
          +--------+                  
      |   | page 4 |               
      |   +--------+                
      |   | page 3 |              
area_1|   +--------+                
      |   | EVICTED|                 
      |   +--------+                 
      |   | EVICTED|  <-0x1D4C000  
          +--------+  


    */      

    char first_byte_area_1 = area_1[0];
    /* area_1 was evicted before. page fault handling will fetch its data from disc.  */
    /*
memory layout diagram:
      
   
          +--------+  
          | area_2 |  <-0x1D5C000
          +--------+  
          | .....  |
          +--------+                  
      |   | page 4 |               
      |   +--------+                
      |   | page 3 |              
area_1|   +--------+                
      |   | EVICTED|                 
      |   +--------+                 
      |   | page 1 |  <-0x1D4C000  
          +--------+  


    */

    if (comparePages(area_1_first_page,area_1)) {
        std::cout << "compare pages SUCCESS" << std::endl;
    }



}