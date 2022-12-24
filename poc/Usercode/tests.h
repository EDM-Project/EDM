#include "../utils/logger.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>

#define PAGE_SIZE 4096

void simpleZeroPageTest();
void simpleEvictionPageTest();
void test_eviction_policy();
void test_simple_flow_eviction();
void end_to_end_test();

static bool comparePages(char* source, char* dest){ 
   if (memcmp(source,dest,PAGE_SIZE) == 0) {
      return true; 
   }
   return false;
}