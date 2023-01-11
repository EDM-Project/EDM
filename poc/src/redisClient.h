#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <sw/redis++/redis++.h>
#include <string>
#include "utils/logger.h"
using namespace std;
 
class RedisClient{

  private:
   
     
  static RedisClient* instancePtr;
         
  RedisClient()
  {
    redis_uri = GetRedisUri();
    LOG(INFO) << "[RedisClient] - get connection: " << redis_uri;
    redis_instance = new sw::redis::Redis(redis_uri); 
  }
   
  public:
  sw::redis::Redis* redis_instance;
  std::string redis_uri;
   
  RedisClient(const RedisClient& obj)
    = delete;
 
  
  static RedisClient* getInstance()
  {
    // If there is no instance of class
    // then we can create an instance.
    if (instancePtr == NULL)
    {
      // We can access private members
      // within the class.
      instancePtr = new RedisClient();
       
      // returning the instance pointer
      return instancePtr;
    }
    else
    {
      return instancePtr;
    }
  }
 
std::string GetRedisUri () {
    std::string redis_uri;
    std::ifstream ReadFile("config.txt");
	if(!ReadFile.is_open()) {
		LOG(ERROR) << "Error: Could not open file.\n";
	}
	std::string word;
	while(ReadFile >> word) {
        if (word == "redis_uri") { 
            ReadFile.ignore(3);
			ReadFile >> word;
            redis_uri  = std::string(word);
        }
	}
	ReadFile.close();
    return redis_uri;
}

void replaceRedisClient() {
    this->redis_uri = GetRedisUri();
    LOG(INFO) << "[RedisClient] - redis uri changed to : " << this->redis_uri;
    this->redis_instance = new sw::redis::Redis(redis_uri); 
}
   
};
 



 #endif