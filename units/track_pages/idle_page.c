// /*
//  * The idle page tracking feature allows to track which memory pages are being accessed by a workload and which are idle
//  * We used the idle page tracking API to for implementing write/read functions (set/get respectively).
//  * Please note that: (from https://www.kernel.org/doc/html/latest/admin-guide/mm/idle_page_tracking.html)
//  * 1. using idle page requires CONFIG_IDLE_PAGE_TRACKING=y
//  * 2. open /sys/kernel/mm/page_idle/bitmap file, should run with root user.
//  * 3. Since the idle memory tracking feature is based on the memory reclaimer logic,
//  *    it only works with pages that are on an LRU list
//  * 4. Only accesses to user memory pages are tracked.
//  *
//  */
// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/mman.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <stdint.h> /* uint64_t  */
// #include <stdbool.h>
// #include <time.h>
// // typedef unsigned long long uint64_t;

// #define PME_PRESENT	(1ULL << 63)
// #define PME_SOFT_DIRTY	(1Ull << 55)

// #ifndef PAGE_SIZE
// #define PAGE_SIZE	4096
// #endif

// typedef struct {
//     uint64_t pfn : 54;
//     unsigned int soft_dirty : 1;
//     unsigned int file_page : 1;
//     unsigned int swapped : 1;
//     unsigned int present : 1;
// } PagemapEntry;

// typedef struct {
//     unsigned int lru : 1;
//     unsigned int active : 1;
//     unsigned int referenced   : 1;
//     unsigned int no_page   : 1;
//     unsigned int slab   : 1;
// } KpageFlagsEntry;

// void print_page_flags(KpageFlagsEntry *entry) {

//     printf("page frame flags: \n");
//     printf("referenced: %d \n",entry->referenced);
//     printf("lru: %d \n",entry->lru);
//     printf("active: %d \n",entry->active);
//     printf("no_page: %d \n",entry->no_page);
//     printf("slab: %d \n",entry->slab);

//     printf("____________ \n");
// }

// void print_entry_flags(PagemapEntry *entry) {

//     printf("entry flags: \n");
//     printf("soft_dirty: %d \n",entry->soft_dirty);
//     printf("file_page: %d \n",entry->file_page);
//     printf("swapped: %d \n",entry->swapped);
//     printf("present: %d \n",entry->present);
//     printf("____________ \n");
// }


// /* Parse the pagemap entry for the given virtual address.
//  * @param[out] entry      the parsed entry
//  * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
//  * @param[in]  vaddr      virtual address to get entry for
//  * @return 0 for success, 1 for failure
//  */
// int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
// {
//     size_t nread;
//     ssize_t ret;
//     uint64_t data;
//     nread = 0;
//     while (nread < sizeof(data)) {
//         ret = pread(pagemap_fd, &data, sizeof(data),
//                     (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
//         nread += ret;
//         if (ret <= 0) {
//             return 1;
//         }
//     }
//     entry->present = (data >> 63) & 1;

//     if (entry->present != 0) {
//         entry->pfn = data & ((1ULL << 55) - 1);
//         entry->soft_dirty = (data >> 55) & 1;
//         entry->file_page = (data >> 61) & 1;
//         entry->swapped = (data >> 62) & 1;
//     } else {
//         perror("page not presented");
//         exit(1);
//     }

//     return 0;
// }

// /* Parse the relevant bits from kpageflags entry for the given pfn.
//  * @param[out] entry      the parsed entry
//  * @param[in]  pfn        the given page frame number
//  * @return 0 for success, 1 for failure
//  */
// int get_page_flags_lru(KpageFlagsEntry *entry, uint64_t pfn)
// {
//     int fd;
//     fd = open("/proc/kpageflags", O_RDONLY);
//     if (fd < 0) {
//         perror("Can't open kpageflags");
//         exit(1);
//     }
//     size_t nread;
//     ssize_t ret;
//     uint64_t data;
//     nread = 0;
//     while (nread < sizeof(data)) {
//         ret = pread(fd, &data, 8,
//                     pfn*8);
//         nread += ret;
//         if (ret <= 0) {
//             return 1;
//         }
//     }
//     entry->referenced = (data >> 2) & 1;
//     entry->lru = (data >> 5) & 1;
//     entry->active = (data >> 6) & 1;
//     entry->no_page = (data >> 20) & 1;
//     entry->slab = (data >> 7) & 1;
//     close(fd);
//     return 0;
// }

// // /* read and print the relevant kpageflags.
// //  * @param[in]  vaddr  page virtual adress of the page
// //  */
//  void read_kflags(uintptr_t vaddr)
//  {
//      int fd;
//     fd = open("/proc/self/pagemap", O_RDONLY);
//      if (fd < 0) {
//         perror("Can't open pagemap");
//          exit(1);
//      }

//      PagemapEntry page_map_entry;
//      KpageFlagsEntry page_flags_entry;
//      pagemap_get_entry(&page_map_entry,fd,vaddr);
//      uint64_t pfn = page_map_entry.pfn;
//      get_page_flags_lru(&page_flags_entry,pfn);
//      print_page_flags(&page_flags_entry);
//      close(fd);
//  }

// /* get page frame number of a given virtual address
//  * @param[in]  vaddr  page virtual adress of the page
//  */
// uint64_t get_pfn_by_addr(uintptr_t vaddr)
// {
//     int fd;
//     fd = open("/proc/self/pagemap", O_RDONLY);
//     if (fd < 0) {
//         perror("Can't open pagemap");
//         exit(1);
//     }

//     PagemapEntry page_map_entry;
//     pagemap_get_entry(&page_map_entry,fd,vaddr);
//     uint64_t pfn = page_map_entry.pfn;
//     close(fd);
//     return  pfn;
// }

// /*
//  * pfn to index of idle page file bitmap
//  *
//  * The bitmap should be read in 8 bytes (64 pages) stride.
//  */
// #define PFN_TO_IPF_IDX(pfn) pfn >> 6 << 3
// #define BIT_AT(val, x)	(((val) & (1ull << x)) != 0)
// #define SET_BIT(val, x) ((val) | (1ull << x))

// typedef unsigned long long u8;

// static uint64_t pread_uint64(int fd, uint64_t index)
// {
//     uint64_t value;
//     off_t offset = index * sizeof(value);

//     if (pread(fd, &value, sizeof(value), offset) != sizeof(value)) {
//     printf("pread offset 0x% failed!",
//                      offset);
//     }

//     return value;

// }

// static bool is_page_idle(int page_idle_fd, uint64_t pfn)
// {
//     uint64_t bits = pread_uint64(page_idle_fd, pfn / 64);

//     return !!((bits >> (pfn % 64)) & 1);
// }

// void set_idle(uint64_t nr_pfns, uint64_t pfns[])
// {
//     int fd;
//     uint64_t pfn;
//     uint64_t entry;
//     u8 i;

//     fd = open("/sys/kernel/mm/page_idle/bitmap", O_RDWR);
//     if (fd < 0) {
//         perror("open bitmap file failed");
//     }

//     for (i = 0; i < nr_pfns; i++) {
//         pfn = pfns[i];
//         entry = 0;

//         if (pread(fd, &entry, sizeof(entry), pfn / 64 * 8) != sizeof(entry))
//         {
//             perror("read bitmap file entry failed");
//         }
//         entry = SET_BIT(entry, pfn % 64);
//         if (pwrite(fd, &entry, sizeof(entry), pfn / 64 * 8) != sizeof(entry))
//         {
//             perror("write to bitmap file entry failed");
//         }

//     }
//     close(fd);
// }

// // void* get_idle_flags(uint64_t nr_pfns, uint64_t pfns[],uint8_t results[])
// // {
// //     int fd;
// //     uint64_t entry, pfn;
// //     u8 i;
// //     clock_t start = clock();

// //     fd = open("/sys/kernel/mm/page_idle/bitmap", O_RDWR);
// //     if (fd < 0)
// //     {
// //         perror("open bitmap file failed");
// //     }
// //     for (i = 0; i < nr_pfns; i++) {
// //         pfn = pfns[i];
// //         entry = 0;
// //         clock_t end = clock();
// //         float seconds = (float) (end - start) / CLOCKS_PER_SEC;
// //         if (pread(fd, &entry, sizeof(entry), pfn / 64 * 8) != sizeof(entry)) {
// //             perror("read bitmap file entry failed");
// //         }
// //        // printf("open file in %.9f",seconds);
// //         results[i] = (uint8_t) BIT_AT(entry, pfn % 64);
// //     }

// //     close(fd);
// // }

// uint8_t get_idle_flag(uint64_t pfn)
// {
//     int fd;
//     uint64_t entry, pfn;
//     u8 i;
    

//     fd = open("/sys/kernel/mm/page_idle/bitmap", O_RDWR);
//     if (fd < 0)
//     {
//         perror("open bitmap file failed");
//     }
    
//     entry = 0;
//     if (pread(fd, &entry, sizeof(entry), pfn / 64 * 8) != sizeof(entry)) {
//         perror("read bitmap file entry failed");
//     }
//     return (uint8_t) BIT_AT(entry, pfn % 64);
    

//     close(fd);
// }