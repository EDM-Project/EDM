#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <sw/redis++/redis++.h>
#include <string>
#include "utils/logger.h"
using namespace std;
 
class RedisClient{

  private:
   
  // member variables
  std::string redis_uri;
  
     
  static RedisClient* instancePtr;
         
  // Default constructor
  RedisClient()
  {
    redis_uri = GetRedisUri();
    redis_instance = new sw::redis::Redis(redis_uri); 
  }
   
  public:
  sw::redis::Redis* redis_instance;
   
  // deleting copy constructor
  RedisClient(const RedisClient& obj)
    = delete;
 
  /*
    getInstance() is a static method that returns an
    instance when it is invoked. It returns the same
    instance if it is invoked more than once as an instance
    of Singleton class is already created. It is static
    because we have to invoke this method without any object
    of Singleton class and static method can be invoked
    without object of class
 
    As constructor is private so we cannot create object of
    Singleton class without a static method as they can be
    called without objects. We have to create an instance of
    this Singleton class by using getInstance() method.
  */
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
      // if instancePtr != NULL that means
      // the class already have an instance.
      // So, we are returning that instance
      // and not creating new one.
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
    LOG(DEBUG) << "[RedisClient] - redis uri changed to : " << this->redis_uri;
    this->redis_instance = new sw::redis::Redis(redis_uri); 
}
   
};
 



 #endif