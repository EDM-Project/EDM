The project currently is defined by two phases, currently the project has come to the end of first phase. Phases 2-5 are subject to change.
 
## First Phase
 
A single app instance with a disc - the design in this phase is single-threaded, and the reason behind it is to show a proof of concept (about the flows - <flow>). That essentially means that there would be a single page-fault handled at a given moment at the most, and when evictions are executed (by LPET), they block user-code (for more info - <Lpet>).
User-code is compiled together with EDM code, and is assumed to be written in C/C++ (for more assumptions - <Assumptions>).
 
## Second Phase
 
A single app instance with a single external memory units (AKA EMU). Instead of evicting to disc, DMS is now part of an EMU, which has its own memory, to which pages are to be evicted.
EMU has three main modules:
1.	DMS - Disc management server
2.	XPET - external low priority eviction thread, handles store requests which are triggered by app
3.	Memory - where pages are to be stored to / loaded from
 
## Third Phase
 
A single app instance with multiple external memory units. The major change is that pages can be evicted to each of the EMUs. In this phase, a page is expected to be stored only in a single EMU.
 
## Fourth Phase
 
Several app instances with several EMUs.
 
## Fifth Phase
 
One app instance with a replicated external memory server - the phase is aimed to solve the delay involved with page request from EMU. App sends request to multiple identical EMUs (in the sense that they store the exact same pages) and waits for the first one to arrive.