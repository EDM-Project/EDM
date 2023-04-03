#include "RedisManagement.h"

int main(int argc, char** argv) {
    printf("inside main function\n");
    initializeServer((char*)"hi",(char*)"5554");
    printf("After initialization\n");
}