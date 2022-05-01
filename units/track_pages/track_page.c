
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h> /* uint64_t  */
typedef unsigned long long u64;

#define PME_PRESENT	(1ULL << 63)
#define PME_SOFT_DIRTY	(1Ull << 55)

#define PAGES_TO_TEST	1
#ifndef PAGE_SIZE
#define PAGE_SIZE	4096
#endif

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
    unsigned int unevictable   : 1;
} KpageFlagsEntry;

// inner functions
void print_page_flags(KpageFlagsEntry *entry) {

    printf("page frame flags: \n");
    printf("referenced: %d \n",entry->referenced);
    printf("lru: %d \n",entry->lru);
    printf("active: %d \n",entry->active);
    printf("no_page: %d \n",entry->no_page);
    printf("unevictable: %d \n",entry->unevictable);

    printf("____________ \n");
}

void print_entry_flags(PagemapEntry *entry) {

    printf("entry flags: \n");
    printf("soft_dirty: %d \n",entry->soft_dirty);
    printf("file_page: %d \n",entry->file_page);
    printf("swapped: %d \n",entry->swapped);
    printf("present: %d \n",entry->present);
    printf("____________ \n");
}


/* Parse the pagemap entry for the given virtual address.
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
    size_t nread;
    ssize_t ret;
    uint64_t data;
    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, &data, sizeof(data),
                    (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->present = (data >> 63) & 1;

    if (entry->present != 0) {
        entry->pfn = data & 0x007fffffffffffff;
        entry->soft_dirty = (data >> 55) & 1;
        entry->file_page = (data >> 61) & 1;
        entry->swapped = (data >> 62) & 1;
    } else {
        perror("page not presented");
        exit(1);
    }

    return 0;
}

/* Parse the relevant bits from kpageflags entry for the given pfn.
 * @param[out] entry      the parsed entry
 * @param[in]  pfn        the given page frame number
 * @return 0 for success, 1 for failure
 */
int get_page_flags_lru(KpageFlagsEntry *entry, uint64_t pfn)
{
    int fd;
    fd = open("/proc/kpageflags", O_RDONLY);
    if (fd < 0) {
        perror("Can't open kpageflags");
        exit(1);
    }
    size_t nread;
    ssize_t ret;
    uint64_t data;
    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(fd, &data, 8,
                    pfn*8);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->referenced = (data >> 2) & 1;
    entry->lru = (data >> 5) & 1;
    entry->active = (data >> 6) & 1;
    entry->no_page = (data >> 20) & 1;
    entry->unevictable = (data >> 18) & 1;
    close(fd);
    return 0;
}



/* read and print the relevant kpageflags.
 * @param[in]  vaddr  page virtual adress of the page
 */
void read_kflags(uintptr_t vaddr)
{
    int fd;
    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0) {
        perror("Can't open pagemap");
        exit(1);
    }

    PagemapEntry page_map_entry;
    KpageFlagsEntry page_flags_entry;
    pagemap_get_entry(&page_map_entry,fd,vaddr);
    uint64_t pfn = page_map_entry.pfn;
    get_page_flags_lru(&page_flags_entry,pfn);
    print_page_flags(&page_flags_entry);
    close(fd);
}


int main(void)
{
	char *mem;

	mem = mmap(NULL, PAGES_TO_TEST * PAGE_SIZE,
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    if(mem == MAP_FAILED)
    {
        printf("Memory allocation failed!");
        exit(1);
    }
    
    //first touch - write
    mem[1] = 'c';
    printf("go to sleep - 2  sec\n");
    sleep(2);
    read_kflags(mem);

    //second touch - read
    char c = mem[1];
    printf("go to sleep - 2  sec\n");
    sleep(2);
    read_kflags(mem);


	return 0;
}
