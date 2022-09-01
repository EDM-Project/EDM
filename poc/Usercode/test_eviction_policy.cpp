#include "tests.h"
/**
 * @brief  test for validate that lpet evicts cold pages
 * in this case, high_threshold= 20 , low_threshold= 15 
 * 
 */
void test_eviction_policy() { 

   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_1[i] = 'x';
   }

   char* area_2 = (char*) mmap( (void*)0x1D51000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_2[i] = 'x';
   }

   char* area_3 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_3[i] = 'x';
   }

   char* area_4 = (char*) mmap( (void*)0x1E19000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_4[i] = 'x';
   }
   // after 4 allocations, the memory layout:
   /*
      +--------+   
      |        |  
      |        |
      | Area 4 |  
      |        |
      |        |  <-0x1E19000
      +--------+  
      |        |
      |        |  
      | Area 3 |  
      |        |  
      |        |  <-0x1E14000
      +--------+  
      :  ..... |    
      +--------+  
      |        |
      |        |    
      | Area 2 |
      |        |  
      |        |  <- 0x1D51000 
      +--------+  
      |        |  
      |        |  
      | Area 1 |
      |        |  
      |        |  <-0x1D4C000
      +--------+  

   */

   usleep(100000); 

   char* area_5 = (char*) mmap( (void*)0x1E20000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
   
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_5[i] = 'x';
   }
   // area 5 first page should trig lpet
   // expected memory layout:
   /*
      +--------+   
      |        |  
      |        |
      | Area 5 |  
      |        |
      |        |  <-0x1E20000
      +--------+   
      |        |  
      |        |
      | Area 4 |  
      |        |
      |        |  <-0x1E19000
      +--------+  
      |        |
      |        |  
      | Area 3 |  
      |        |  
      |        |  <-0x1E14000
      +--------+  
      :  ..... |    
      +--------+  
      |        |
      |        |    
      | Area 2 |
      |        |  
      |        |  <- 0x1D51000 
      +--------+  
      |        |  
      | Area 1 |  
      | EVICTED|
      |        |    
      |        |  <-0x1D4C000
      +--------+  
   */
   usleep(70000); 


   //touch pages in area 2 
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_2[i] = 'z';
   }

   //touch pages in area 3
   for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
      area_3[i] = 'z';
   }
   usleep(10000); 

   // bring back from disc, will trig lpet
   area_1[0] = 'y';
   // expected memory layout:

/*
       +--------+   
      |        |  
      |        |
      | Area 5 |  
      |        |
      |        |  <-0x1E20000
      +--------+   
      |        |  
      |        |
      | Area 4 |  
      | EVICTED|
      |        |  <-0x1E19000
      +--------+  
      |        |
      |        |  
      | Area 3 |  
      |        |  
      |        |  <-0x1E14000
      +--------+  
      :  ..... |    
      +--------+  
      |        |
      |        |    
      | Area 2 |
      |        |  
      |        |  <- 0x1D51000 
      +--------+  
      |        |  
      | Area 1 |  
      | EVICTED|
      +--------+    
      | page_0 |  <-0x1D4C000
      +--------+  

   */

}
