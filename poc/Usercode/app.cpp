
#include "../shared/logger.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>

#define PAGE_SIZE 4096 
#define MREMAP_DONTUNMAP	4

bool comparePages(char* source, char* dest){ 
   if (memcmp(source,dest,PAGE_SIZE) == 0) {
      return true; 
   }
   return false;
}
/**
 * @brief simple end-to-end test to verify flow correctness. 
 * 
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

/**
 * @brief much complicated e2e test with 2 eviction cycles and memory correctness
 * 
 */
void end_to_end_test() {

   //area_1 - start in 0x1D4C000 
   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE *10 , PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *10 ; i++ ) {
      area_1[i] = 'x';
   }
   // save first 5 pages data for future verification.
   char* first_pages_buffer = (char*)malloc(5 * PAGE_SIZE);
   memcpy(first_pages_buffer,area_1,5 *PAGE_SIZE);

   char* area_2 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE *10 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   for (int i =0; i < PAGE_SIZE *10 ; i++ ) {
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
   char* area_3 = (char*) mmap( (void*)0x1E82000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_3[i] = 'z';
   }

   // this page fault should evict all the first 10 pages
   // so the memory layout should looks like this: 
   /*
   .__________.
   .0x1D4C000 .
   .  MAPPED  .
   .    BUT   .
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
   .          .
   .ox1E87000 . 
   .__________.
   .__________.

   */

   // bring back first 5 pages of area_1 

   for (size_t i = 0; i < 5; i++)
   {
      char temp = area_1[i * PAGE_SIZE];
      //check data correctness of these pages.
      if (comparePages(first_pages_buffer+(i* PAGE_SIZE),area_1+(i*PAGE_SIZE))){
         LOG(DEBUG) << "page index: " << i << " test compare page SUCCESS";
      } 
   }
   // now the memory layout should looks like this: exactly 20 pages
/*
   .__________.
   .0x1D4C000 .
   .          .
   .          .
   .0x1D51000 .
   .0x1D52000 .
   .  MAPPED  .
   .    BUT   .
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
   .          .
   .0x1E87000 . 
   .__________.
   .__________.

   */

   //create forth mapping which cause another eviction cycle
   char* area_4 = (char*) mmap( (void*)0x1E89000, PAGE_SIZE *1 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0); 

   area_4[0] = 'w';
   
   //now memory layout should look like this:

   /*
   .__________.
   .0x1D4C000 .
   .          .
   .          .
   .0x1D51000 .
   .0x1D52000 .
   .  MAPPED  .
   .    BUT   .
   .   EMPTY  .
   .   DATA   .
   .0x1D56000 .
   .__________.
   .
   .__________.
   .0x1E14000 .
   .  MAPPED  .
   .    BUT   .
   .   EMPTY  .
   .   DATA   .  
   .0X1E1E000 .
   .__________.
   .          .
   .__________.
   .0x1E82000 . 
   .          .
   .0x1E87000 . 
   .__________.
   .0X1E87000
   .__________.

   */


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
   end_to_end_test();

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