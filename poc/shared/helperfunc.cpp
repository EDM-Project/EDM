#include "helperfunc.h"


std::string convertToHexRep(unsigned long long vaddr) {
    std::ostringstream temp_stream;
    temp_stream << "0x" << std::setfill('0') << std::setw(8) << std::hex << vaddr; /* convert vaddr to hex correct format*/
    std::string str_vaddr = temp_stream.str();
    return str_vaddr;
}