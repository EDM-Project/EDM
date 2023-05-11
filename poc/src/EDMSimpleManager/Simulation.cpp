#include "RedisManagement.h"


/* compilation command: gcc Simulation.cpp RedisManagement.cpp -o sim -lstdc++*/
int main(int argc, char** argv) {
    printf("inside main function\n");
    /* initializing the cluster's server*/
    initializeServer((char*)"cluster-nodes/5001/5001.conf",(char*)"5554");
    initializeServer((char*)"cluster-nodes/5001/5001.conf",(char*)"5554");
    initializeServer((char*)"cluster-nodes/5002/5002.conf",(char*)"5554");
    initializeServer((char*)"cluster-nodes/5003/5003.conf",(char*)"5554");
    initializeServer((char*)"cluster-nodes/5004/5004.conf",(char*)"5554");
    initializeServer((char*)"cluster-nodes/5005/5005.conf",(char*)"5554");
    printf("After initialization\n");
}