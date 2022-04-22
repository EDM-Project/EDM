/*
The soft-dirty is a bit on a PTE which helps to track which pages a task writes to.
In order to do this tracking one should
1. Clear soft-dirty bits from the task’s PTEs.
   This is done by writing “4” into the /proc/PID/clear_refs file of the task in question.
2. Wait some time.
3. Read soft-dirty bits from the PTEs.
This is done by reading from the /proc/PID/pagemap.
The bit 55 of the 64-bit qword is the soft-dirty one. If set, the respective PTE was written to since step 1.
 */


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

#define PAGES_TO_TEST	3
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

/* Parse the pagemap entry for the given virtual address.
 *
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
    entry->pfn = data & (((uint64_t)1 << 54) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;
    return 0;
}

void print_entry_flags(PagemapEntry *entry) {

    printf("entry flags: \n");
    printf("soft_dirty: %d \n",entry->soft_dirty);
    printf("file_page: %d \n",entry->file_page);
    printf("swapped: %d \n",entry->swapped);
    printf("present: %d \n",entry->present);
    printf("____________ \n");
}

static inline char map_p(u64 map)
{
	return map & PME_PRESENT ? 'p' : '-';
}


static void clear_refs(void)
{
	int fd;
	char *v = "4";

	fd = open("/proc/self/clear_refs", O_WRONLY);
	if (write(fd, v, 3) < 3) {
		perror("Can't clear soft-dirty bit");
		exit(1);
	}
	close(fd);
}

int main(void)
{
	char *mem;

	mem = mmap(NULL, PAGES_TO_TEST * PAGE_SIZE,
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);

	mem[0] = 'c';
    int fd;
    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0) {
        perror("Can't open pagemap");
        exit(1);
    }
    //expecting dirty and presented =1
    PagemapEntry entry;
    pagemap_get_entry(&entry,fd,mem);
    print_entry_flags(&entry);

    //expecting dirty = 0
	clear_refs();

    pagemap_get_entry(&entry,fd,mem);
    print_entry_flags(&entry);

    //expecting dirty get back to 1

    mem[0] = 'c';
    pagemap_get_entry(&entry,fd,mem);
    print_entry_flags(&entry);
	return 0;
}