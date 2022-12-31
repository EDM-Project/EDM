/*
 * The idle page tracking feature allows to track which memory pages are being accessed by a workload and which are idle
 * We used the idle page tracking API to for implementing write/read functions (set/get respectively).
 * Please note that: (from https://www.kernel.org/doc/html/latest/admin-guide/mm/idle_page_tracking.html)
 * 1. using idle page requires CONFIG_IDLE_PAGE_TRACKING=y
 * 2. open /sys/kernel/mm/page_idle/bitmap file, should run with root user.
 * 3. Since the idle memory tracking feature is based on the memory reclaimer logic,
 *    it only works with pages that are on an LRU list
 * 4. Only accesses to user memory pages are tracked.
 *
 */
#ifndef IDLEPAGE_H
#define IDLEPAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h> /* uint64_t  */
#include <stdbool.h>
#include <time.h>
#include <string>
#include "../utils/logger.h"


// typedef unsigned long long uint64_t;

#define PME_PRESENT	(1ULL << 63)
#define PME_SOFT_DIRTY	(1Ull << 55)
#define PRINT_AS_HEX(ADDR) std::hex << "0x" << ADDR << std::dec

#ifndef PAGE_SIZE
#define PAGE_SIZE	4096
#endif

/*
 * pfn to index of idle page file bitmap
 *
 * The bitmap should be read in 8 bytes (64 pages) stride.
 */
#define PFN_TO_IPF_IDX(pfn) pfn >> 6 << 3
#define BIT_AT(val, x)	(((val) & (1ull << x)) != 0)
#define SET_BIT(val, x) ((val) | (1ull << x))

typedef unsigned long long u8;

typedef struct {
    uint64_t pfn : 54;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

typedef struct {
    unsigned int lru : 1;
    unsigned int active : 1;
    unsigned int referenced   : 1;
    unsigned int no_page   : 1;
    unsigned int slab   : 1;
} KpageFlagsEntry;

void print_page_flags(KpageFlagsEntry *entry) ;

void print_entry_flags(PagemapEntry *entry);

/* Parse the pagemap entry for the given virtual address.
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr);


/* Parse the relevant bits from kpageflags entry for the given pfn.
 * @param[out] entry      the parsed entry
 * @param[in]  pfn        the given page frame number
 * @return 0 for success, 1 for failure
 */
int get_page_flags_lru(KpageFlagsEntry *entry, uint64_t pfn);

// /* read and print the relevant kpageflags.
//  * @param[in]  vaddr  page virtual adress of the page
//  */
void read_kflags(uintptr_t vaddr, pid_t pid = -1);


/* get page frame number of a given virtual address
* @param[in]  vaddr  page virtual adress of the page
*/
uint64_t get_pfn_by_addr(uintptr_t vaddr, pid_t pid = -1);


static uint64_t pread_uint64(int fd, uint64_t index);

static bool is_page_idle(int page_idle_fd, uint64_t pfn);

void set_idle_pages(uint64_t nr_pfns, uint64_t pfns[]);

void* get_idle_flags(uint64_t nr_pfns, const uint64_t pfns[],uint8_t results[]);

bool isPageSwappedOrPresent(pid_t pid, uint64_t vaddr);

std::string getProcPagemapPath(pid_t pid = -1);

#endif