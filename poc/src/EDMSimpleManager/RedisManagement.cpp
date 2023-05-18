#include "RedisManagement.h"
/* we assume that Redis is installed properly at the instance where
we run this file */

/*command for compiling the 'regular' code: 
g++ -std=c++17 utils/*.cpp utils/ptrace_do/libptrace_do.c utils/ptrace_do/parse_maps.c DmHandler/* appMonitor.cpp LSPT/*.cpp mapTracker.cpp monitoredAreas.cpp vmaMetadata.h redisClient.cpp -o app -lredis++ -lhiredis -pthread
*/

/* initialize a Redis server with a specified port number and host IP address
This function currently does not take care of security measures.
When one uses this function, he/she must NOT close the process as long as
the server is supposed to work. Forking might help with that*/
void initializeServer(char* pathToConfig, char* port) {
    /* important note: whenever we run this line of code, it createas a NEW server
    within the same port and ip values, thus OVERRIDING the previous/existing one*/
    /* naive way*/
    /*char* directWayArgs[] = {(char*)"redis-server", (char*)"--port ",port,NULL};*/
    /*if (execvp("redis-server", directWayArgs)==-1) {
        printf("failed to open a new redis server\n");
    }*/
    /*exit(0); -> does not cause the termination of the function after server init */
    /* more proper way - using config file*/
    /* the original command is ./redis-server /path/to/redis.conf
    in the current implementation we assume the we get the path to a default config file
    and need to change it*/
    char* configWayArgs[] = {(char*)"redis-server", pathToConfig,NULL};
    if (execvp("redis-server", configWayArgs)==-1) {
        printf("failed to open a new redis server\n");
    }
    printf("end of func\n");
}

/* TODO: realize what important aspects we would like to change in a server and add
them as arguments */
/* We assume that there is a default demo config file called redis.conf in the EDMSimpleManager folder
*/
void setConfigFileForServer() {

}