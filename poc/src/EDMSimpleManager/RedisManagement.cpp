#include "RedisManagement.h"
/* we assume that Redis is installed properly at the instance where
we run this file */


/* initialize a Redis server with a specified port number and host IP address
This function currently does not take care of security measures.
When one uses this function, he/she must NOT close the process as long as
the server is supposed to work. Forking might help with that*/
void initializeServer(char* pathToConfig, char* port) {
    /* important note: whenever we run this line of code, it createas a NEW server
    within the same poort and ip values, thus OVERRIDING the previous/existing one*/
    /* naive way*/
    char* directWayArgs[] = {(char*)"redis-server", (char*)"--port ",port,NULL};
    if (execvp("redis-server", directWayArgs)==-1) {
        printf("failed to open a new redis server\n");
    }
    exit(0);
    /* more proper way - using config file*/
    /* the original command is ./redis-server /path/to/redis.conf
    in the current implementation we assume the we get the path to a default config file
    and need to change it*/
    char* configWayArgs[] = {(char*)"redis-server", pathToConfig,port,NULL};
    if (execvp("redis-server", configWayArgs)==-1) {
        printf("failed to open a new redis server\n");
    }
}