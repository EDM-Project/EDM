#ifndef VMA_METADATA
#define VMA_METADATA

extern "C" { 
#include "utils/ptrace_do/libptrace_do.h"
}
#include <string>

class VmaMetadata { 
public:

    unsigned long start_address;
	unsigned long end_address;
	unsigned int perms;
	unsigned long offset;
	unsigned int dev_major;
	unsigned int dev_minor;
	unsigned long inode;
	std::string pathname;
    
    VmaMetadata(struct parse_maps * entry) { 
        start_address = entry->start_address;
        end_address = entry->end_address;
        perms = entry->perms;
	    offset = entry->offset;
	    dev_major = entry->dev_major;
	    dev_minor = entry->dev_minor;
	    inode= entry->inode;
	    pathname = std::string(entry->pathname);
    }
    

};
#endif