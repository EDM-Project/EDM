#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>

#define PAGE_SIZE 4096

void waitFor(int time_in_seconds) { 
  int cnt = 0; 

  while (cnt < time_in_seconds)
  {
      time_t current_time;
      time(&current_time);
      printf("PID: %d, Current time: %s", getpid(), ctime(&current_time));

      sleep(1);
      cnt++;
  }
}

int main() {

   std::cout<< "[Usercode] : User code main function start running" << std::endl;

    waitFor(7);

   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
    /* 0x1D4C000 is in shared virtual space, so the memory mapping is registered in userfaultfd */

    waitFor(3);
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

   std::cout<< "[Usercode] : area_1[0] " << area_1[0] << std::endl;
   
    waitFor(5);

}