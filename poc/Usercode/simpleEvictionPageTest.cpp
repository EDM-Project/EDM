#include "tests.h"
/**
 * @brief simple test to show eviction of page to disc, and getting this page back.
 *      configuration: high_threshold = 4 , low_threshold = 2
 */
void simpleEvictionPageTest() {
    char* area_1 = (char*) mmap( (void*)0x1D4C000, 4 *PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
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

    char* area_2 = (char*) mmap( (void*)0x1D5C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  

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
        LOG(INFO) << "compare pages SUCCESS";
    }



}