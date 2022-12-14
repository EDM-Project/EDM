EDM Project - Technion

1. [What Is EDM?](#what-is-edm)
2. [What should I use EDM for?](#what-should-i-use-edm-for)
3. [How to use EDM?](#how-to-use-edm)
## What Is EDM?

EDM - External Shared Memory
 
EDM is aimed to create a reliable layer of memory over multiple machines, so a could native application would be able to utilize more than one machine's memory, rather than its own disc, which is a much slower memory unit. This layer essentially allows the application's performance to increase significantly.
 
EDM deals with some of the challenges that derive from distributing memory over multiple machines, among them page faults and evictions, which must create a communication channel between two machines.
 
EDM is part of an Red Hat's project named SpotOS. This project aims to devise and implement a distributed cloud-based operating system that uses unreliable or temporarily available resources to provide reliable and scalable execution experience with high quality-of-service. EDM is a module which is responsible of memory management in the system.
 
One of the major overheads involved in a distributed operating system, is caused when an instance (in AWS known as spot) is reclaimed by the cloud provider due to economic prioritization. In this case, a spot is required to be able to migrate to another machine over the cloud at a given moment and resume execution. This leads us to the basic and major requirement from EDM to save the state of multiple spots' memory at each moment, including page-fault handling, page swaps and caching.
 
For more information please visit [Red Hat's Introduction to SpotOS](https://research.redhat.com/blog/research_project/spotos-a-distributed-cloud-based-operating-system/).


## What should I use EDM for?
You should use EDM in case you are interested in memory management of a given workload, among them:
- Code execution over a distributed system with multiple memory instances over the network
- Caching optimizations for a real-time embedded system with limited resources


## How to use EDM?
**Structure:**

EDM, in its current form (POC - phase 1), provides a wrapper system to code. The code base is divided to modules, where user code can be put in Usercode directory.

In fact, EDM consists of 3 processes:

-   MPI Manager- which spawns 2 processes - Client and DMS.
-   Client - (with usercode)
-   DMS

DMS and Client should be compiled before manage, so manager could spawn the two and enable easy communication between them.

**Compile:**
```
mpic++ shared/* DMS/mpiDms/*.cpp DMS/dms.cpp DMS/dmsInit.cpp DMS/spt.cpp -o  dms 
mpic++ shared/* CLIENT/mpiClient/* CLIENT/DmHandler/* CLIENT/client.cpp Usercode/*.cpp CLIENT/LSPT/*.cpp -o  app
gcc -Wall -fPIC -shared -o mymmap.so CLIENT/mmap.c -ldl
mpic++ manager.cpp -o manager

```

**Run command:**

```
sudo mpirun -n 1 <arguments> --allow-run-as-root manager
```


The command 'run mpi manager', spawns Client process and DMS process with their arguments and the necessary configuration to communicate.
sudo limitation resulting from the usage of "idle page tracking". 
Any arguments can be passed in this run command.

**Configuration files:**

There are two configuration files.

-   Client_config.txt in Client side

Example:

```
start_addr = 0x1B58000 // starting address of the shared memory space

end_addr = 0x1F40000 // ending address of the shared memory space

low_threshold = 10 // low threshold for lpet eviction

high_threshold = 20
```

-   Dms_config.txt in DMS side

Example:
```
start_addr = 0x1B58000
```

Note: DMS stores pages in disc relatively to start_addr.

**Output and Logging:**

EDM uses an open source project for thread-safe logger called [CPPLOGGER](https://github.com/samckittrick/CPPLogge).The logger contains 5 log levels (debug, info, warn, error, fatal). All the events and the interactions between modules and processes in EDM are being reported to logger.

Each logging is composed as follows: [timestamp] : [logLevel] - [module] - [message].

For example:
```
[Tue Aug 30 2022 18:40:33.934]:[Info]-[DmHandler] - send request for the page in address 0x1d4e000 from DMS
```

**Notes**:

-   We uses LD_PRELOAD to override the symbol 'mmap' in Client process. The usage of preload mmap and the motivation behind it is described here <link>
