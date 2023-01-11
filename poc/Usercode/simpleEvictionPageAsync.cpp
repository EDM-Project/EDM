#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "logger.h"

#define PAGE_SIZE 4096

void waitFor(int time_in_seconds, std::string message = std::string(), bool to_print = false) { 
  int cnt = 0; 

  while (cnt < time_in_seconds)
  {
      if (to_print) { 
        LOG(INFO) << "[Usercode] - PID: "<< getpid() << " " << message;

      }
      sleep(1);
      cnt = cnt +1;
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
    
    LOG(INFO) <<  "[Usercode] : touch 2 first pages (0x1D4C000 ,0x1D4D000)";             
    for (int i=0 ; i < 2 *PAGE_SIZE ; i++) {
        area_1[i] = 'x';
    }
    waitFor(2); 
    LOG(INFO) <<  "[Usercode] : touch 2 last pages (0x1D4E000 ,0x1D4F000)";             
    for (int i= 2*PAGE_SIZE ; i < 4 *PAGE_SIZE ; i++) {
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
          | .....  |l
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

    LOG(INFO) <<  "[Usercode] - touch page in address: 0x1D5C000";
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

    waitFor(10,"workload...time to migrate redis",true);   

    LOG(INFO) <<  "[Usercode] - touch the first page (0x1D4C000)";
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