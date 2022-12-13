#ifndef HELPERFUNC_H
#define HELPERFUNC_H
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

std::string convertToHexRep(unsigned long long vaddr);


#endif