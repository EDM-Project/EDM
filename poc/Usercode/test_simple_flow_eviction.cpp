#include "tests.h"

/**
 * @brief simple end-to-end test to verify flow correctness. with one lpet cycle
 *        config: high_treshold=20 , low_threshold=10
 */
void test_simple_flow_eviction() {

   //area_1 - start in 0x1D4C000 
   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE *10 , PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *10 ; i+= PAGE_SIZE ) {
      area_1[i] = 'x';
   }
   // save first_page data for future verification.
   char* first_page_buffer = (char*)malloc(PAGE_SIZE);
   memcpy(first_page_buffer,area_1,PAGE_SIZE);

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

   char temp = area_1[0]; // this line should trigged page fault, which will be solved by getting the page from dms
   LOG(DEBUG) <<  "area 1 first byte is: " << area_1[0];
   LOG(DEBUG) <<  "first_page_buffer first byte is: " << first_page_buffer[0];
   
   if (comparePages(area_1,first_page_buffer)) {
      LOG(DEBUG) << "comaparePages - success "; 

   }
   else { 
      LOG(DEBUG) << "comaparePages - failed ";
   }
}
