## Example #1:

### Course of Action
EDM identifies a page-fault raised by app, to a new page (first access), and returns a new zero-page to app.

### Configuration
Address space registered to Userfaultfd - 0x1B58000 - 0x1F40000

### App code

```
#define PAGE_SIZE 4096

void simpleZeroPageTest() {

   LOG(DEBUG) << "[Usercode] : User code main function start running" ;

   char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
   area_1[0] = 'x';
   LOG(DEBUG)<< "[Usercode] : area_1[0] " << area_1[0] ;  

}
```

### Output
```
--------------------------------------------------------------------------
[Thu Sep 01 2022 12:53:54.156]:[Debug]-[EDM CLIENT] -  INIT
[Thu Sep 01 2022 12:53:54.320]:[Debug]-[DMS] - DMS ready for serving requests
[Thu Sep 01 2022 12:53:54.321]:[Debug]-[Usercode] : User code main function start running
[Thu Sep 01 2022 12:53:54.321]:[Info]-
--------------START HADNLING PAGE FAULT--------------
[Thu Sep 01 2022 12:53:54.322]:[Info]-[DmHandler] - UFFD_EVENT_PAGEFAULT in address = 0x1d4c000
[Thu Sep 01 2022 12:53:54.322]:[Info]-[DmHandler] - send request for the page in address 0x1d4c000 from DMS
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[MpiClient] - send DmHandler's request for the page in address 0x1d4c000
[Thu Sep 01 2022 12:53:54.328]:[Info]-[DMS] - get request to send content of page in address: 0x1d4c000
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[DMS] - page accessed first time, set info - new_page
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[DMS] - SPT updated. Current state: 
[Thu Sep 01 2022 12:53:54.328]:[Info]-DMS- Print SPT STATE
-----START SPT ----
address : 0x1d4c000  location : INSTANCE_0
------END SPT-----
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[DMS] - page in address: 0x1d4c000 sent to app
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[MpiClient] - get DMS's response for the page in address 0x1d4c000
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[MpiClient] - RequestPageFromDMS succeeded. 
[Thu Sep 01 2022 12:53:54.328]:[Info]-[DmHandler] - received ack for page in address : 0x1d4c000 (first access)
[Thu Sep 01 2022 12:53:54.328]:[Info]-[DmHandler] - copying zero page to address : 0x1d4c000
[Thu Sep 01 2022 12:53:54.328]:[Info]-
--------------FINISH HADNLING PAGE FAULT--------------


[Thu Sep 01 2022 12:53:54.328]:[Debug]-[Usercode] : area_1[0] x
[Thu Sep 01 2022 12:53:54.328]:[Debug]-[EDM CLIENT] - SHUTDOWN!
```

### Example Review

1. As expected, mmap in user-code triggers a page-fault, then MpiClient sends a request for the page.
2. When DMS detects this is a new page, it returns zero-page to client, and updates SPT.
3. The moment Client gets back the page it continues execution, and eventually, as a validation step, it prints the modified byte - 'x'.


## Example #2:

### Course of Action
EDM evicts two pages to disc after high_threshold is reached. Then the app accesses one of the pages, which leads to a page fault, that is resolved by fetching the page back from the disc to the instance.

### Configuration
Address space registered to Userfaultfd - 0x1B58000 - 0x1F40000, high_threshold = 4, low_threshold = 2

### App code

```
#define PAGE_SIZE 4096
void simpleEvictionPageTest() {

    char* area_1 = (char*) mmap( (void*)0x1D4C000, 4 *PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
    for (int i=0 ; i < 4 *PAGE_SIZE ; i++) {
        area_1[i] = 'x';
    }
    /* save area_1 first page in buffer for validation */
    char* area_1_first_page = (char*)malloc(PAGE_SIZE);
    memcpy(area_1_first_page,area_1,PAGE_SIZE);
    
    char* area_2 = (char*) mmap( (void*)0x1D5C000, PAGE_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);  
    area_2[0] = 'z';
    char first_byte_area_1 = area_1[0];
    if (comparePages(area_1_first_page,area_1)) {
        LOG(INFO) << "compare pages SUCCESS";
    }
}
```
User-code allocates area in size of 4 pages. When the fifth page in the system (area 2) is allocated, LPET is triggered, evicts 2 pages (to reach low_threshold), and the instance fetches the new page (area 2). Finally, the first page (of area 1) is accessed, which leads to a page-fault that is resolved by fetching the previously-evicted page back to instance.


### Output

Let's see the expected memory layout (of Client) side by side with the output.
The full output file is located under "tests_output" folder, as "simple_eviction_page_test_debug.txt"

**Step 1:**

At first, the code allocates an area of 4 pages and accesses all of them.
The expected memory layout would be:

![](https://www.planttext.com/api/plantuml/png/bPF13e8m38RlVOhT2MGeHmya-XoIwXP6H2LiAWSUNfaG6D24_PUrJVFrpvf2gWtMvdKl0VwXA0mY032h_kaKXBq2cweYLmo5gANMawvs4ZZb7_nwi-MMQAVgRyA4qnvrSXapmuvW7DEo2dF9ad7cieT6BK7xEGYmbfZ5lfpTmj16Iokik4iE8M_YKtBad9EBzVbJObtdJUjL4gkr-XZuJNMDNxHcbF5D0846FG6iV7vM3o8oiZ7aDmw7big766xF9lOcuGi0)

the current state of DMS from the output logging file: (after 4-page faults)
```
-----START SPT ----
address : 0x1d4c000  location : INSTANCE_0
address : 0x1d4d000  location : INSTANCE_0
address : 0x1d4e000  location : INSTANCE_0
address : 0x1d4f000  location : INSTANCE_0
------END SPT-----
```

**Step 2:**

The code allocates a new area and accesses another page. This should wake up lpet thread. In this case, all 4 pages are "hot", so lpet will evict the first 2 pages. After that, the new page (of area 2) will be added to the system.

Expected memory layout would be (without .text, .data etc. sections): 

![](https://www.planttext.com/api/plantuml/png/ut8eBaaiAYdDpU7YWZ1JCaiI4xcuk1J004g1WBOk56YZ2DSe91Qb9iOR0HaADhe65OOkfiu61WR4QDG30I03Xrfqm0NJ1y85YUcf2YPW3Wurk8t2DCWOtI0kiASCIJ37DSpJESJL1Tr1H3e9brcG43K1XIYXoE1oICrBWSKUc0EELIu0)

In log file:

When handling the fifth page (0x1d5c000) we can see that dm-handler wakes up lpet thread. 
LPET start running, and shows the current state of LSPT (data structure of the pages in  the instance), there are 4 pages with is_idle=0. As decribed in LPET section, it will evict the first two pages.
```
--------------START HADNLING PAGE FAULT--------------
[Thu Sep 01 2022 19:24:57.466]:[Info]-[DmHandler] - UFFD_EVENT_PAGEFAULT in address = 0x1d5c000
[Thu Sep 01 2022 19:24:57.466]:[Info]-[DmHandler] - reached high threshold, waking up lpet
...
[Thu Sep 01 2022 19:24:57.466]:[Info]-
-----------------START LPET-----------------

[Thu Sep 01 2022 19:24:57.466]:[Info]-[Lpet] - start address: 0x1d4c000
[Thu Sep 01 2022 19:24:57.466]:[Info]-page list state: 
-----START LSPT ----
address : 0x1d4c000  pfn : 45052  is_idle: 0
address : 0x1d4d000  pfn : 577513  is_idle: 0
address : 0x1d4e000  pfn : 116963  is_idle: 0
address : 0x1d4f000  pfn : 234643  is_idle: 0
-----END LSPT ----
```
Eventually, after pages eviction, DMS state is:
```
-----START SPT ----
address : 0x1d4c000  location : DISK
address : 0x1d4d000  location : DISK
address : 0x1d4e000  location : INSTANCE_0
address : 0x1d4f000  location : INSTANCE_0
address : 0x1d5c000  location : INSTANCE_0
------END SPT-----
```
The up-to-date location of the pages that were evicted is on disc.

**Step 3:**

The code accesses the first page of area 1 (that were evicted lately). This will end up with a page fault handling that will fetch the page from disc. 

Expected memory layout:

![](https://www.planttext.com/api/plantuml/png/ut8eBaaiAYdDpU7YWZ1JCaiI4xcuk1J004g1WBOk56YZ2DSe91Qb9iOR0HaADhe65OOkfiu61WR4QDG30I03Xrfqm0NJ1y85YUcf2YPW3Wurk8t2DCWOtI0kiASCIJ37DSpJESJL1Tr1H3e9YviC4I5g0WfHGf71vP2QbmABFJ077AjS0000)

```
--------------START HADNLING PAGE FAULT--------------
[Thu Sep 01 2022 19:24:57.500]:[Info]-[DmHandler] - UFFD_EVENT_PAGEFAULT in address = 0x1d4c000
[Thu Sep 01 2022 19:24:57.500]:[Info]-[DmHandler] - send request for the page in address 0x1d4c000 from DMS
[Thu Sep 01 2022 19:24:57.500]:[Debug]-[MpiClient] - send DmHandler's request for the page in address 0x1d4c000
[Thu Sep 01 2022 19:24:57.517]:[Info]-[DMS] - get request to send content of page in address: 0x1d4c000
[Thu Sep 01 2022 19:24:57.517]:[Debug]-[DMS] - read page address: 0x1d4c000 from disk
[Thu Sep 01 2022 19:24:57.517]:[Debug]-[DMS] - SPT updated. Current state: 
[Thu Sep 01 2022 19:24:57.517]:[Info]-DMS- Print SPT STATE
-----START SPT ----
address : 0x1d4c000  location : INSTANCE_0
address : 0x1d4d000  location : DISK
address : 0x1d4e000  location : INSTANCE_0
address : 0x1d4f000  location : INSTANCE_0
address : 0x1d5c000  location : INSTANCE_0
------END SPT-----
[Thu Sep 01 2022 19:24:57.517]:[Debug]-[DMS] - page in address: 0x1d4c000 sent to app
[Thu Sep 01 2022 19:24:57.517]:[Debug]-[MpiClient] - get DMS's response for the page in address 0x1d4c000
[Thu Sep 01 2022 19:24:57.517]:[Debug]-[MpiClient] - RequestPageFromDMS succeeded. 
[Thu Sep 01 2022 19:24:57.517]:[Info]-[DmHandler] - received ack for page in address : 0x1d4c000 (previously accessed)
[Thu Sep 01 2022 19:24:57.517]:[Info]-[DmHandler] - copying page content from DMS to address : 0x1d4c000
[Thu Sep 01 2022 19:24:57.518]:[Info]-
--------------FINISH HADNLING PAGE FAULT--------------
[Thu Sep 01 2022 19:24:57.518]:[Info]-compare pages SUCCESS
```
This time, DMS fetches the page from disc, and dm-handler resolves the page fault by copying the existing page to 0x1d4c000 (first page).
Finally, the code compares the data (first page) before and after eviction. The comparison succeeds.

## Example #3:

### Course of Action
This test aims to show the expected behavior on a relatively big amount of pages, including 2 LPET eviction cycles from different memory locations.
* User allocates 4 areas in size of 5 pages each (20 pages in total, which is high_thresh).
* In the following 5-page allocation (area_5) LPET wakes up and evicts first 5 pages to disc (area_1). In the end of this step - 20 pages are in instance.
* Then user accesses all 10 pages in area_2, area_3, which leads to marking them as 'hot',
* User touches first page of area_1, which triggers second LPET eviction. This time, LPET looks for idle-pages starting from area_2, keeps area_2, area_3 as they are hot, and evicts area_4.  

### Configuration
Address space registered to Userfaultfd - 0x1B58000 - 0x1F40000, high_threshold = 20, low_threshold = 15

### App code

```
void test_eviction_policy() {

char* area_1 = (char*) mmap( (void*)0x1D4C000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE, 
			        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_1[i] = 'x';
}

char* area_2 = (char*) mmap( (void*)0x1D51000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_2[i] = 'x';
}

char* area_3 = (char*) mmap( (void*)0x1E14000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
			        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_3[i] = 'x';
}

char* area_4 = (char*) mmap( (void*)0x1E19000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_4[i] = 'x';
}

usleep(100000);

char* area_5 = (char*) mmap( (void*)0x1E20000, PAGE_SIZE *5 , PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_5[i] = 'x';
}

usleep(70000);

for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_2[i] = 'z';
}

for (int i =0; i < PAGE_SIZE *5 ; i++ ) {
	area_3[i] = 'z';
}

usleep(10000);

area_1[0] = 'y';
   ```
   

### Output

Let's see the expected memory layout (of Client) side by side with the output.
The full output file is located under "tests_output" folder, as "test_eviction_policy_info.txt"

**Step 1:**

At first, the code allocates 4 areas of 5 pages each, and accesses all of them.

<u> Expected Memory Layout:

![image](https://user-images.githubusercontent.com/63795552/188273738-ac6fc26a-da54-4b5d-993d-8e0b7a849692.png)


The current state of DMS from the output logging file, after 20-page faults:
```
-----START SPT ----
address : 0x1d4c000 location : INSTANCE_0
address : 0x1d4d000 location : INSTANCE_0
address : 0x1d4e000 location : INSTANCE_0
address : 0x1d4f000 location : INSTANCE_0
address : 0x1d50000 location : INSTANCE_0
address : 0x1d51000 location : INSTANCE_0
address : 0x1d52000 location : INSTANCE_0
address : 0x1d53000 location : INSTANCE_0
address : 0x1d54000 location : INSTANCE_0
address : 0x1d55000 location : INSTANCE_0
address : 0x1e14000 location : INSTANCE_0
address : 0x1e15000 location : INSTANCE_0
address : 0x1e16000 location : INSTANCE_0
address : 0x1e17000 location : INSTANCE_0
address : 0x1e18000 location : INSTANCE_0
address : 0x1e19000 location : INSTANCE_0
address : 0x1e1a000 location : INSTANCE_0
address : 0x1e1b000 location : INSTANCE_0
address : 0x1e1c000 location : INSTANCE_0
address : 0x1e1d000 location : INSTANCE_0
------END SPT-----
```

**Step 2:**

The code allocates a new 5-page area and accesses it. This should wake up LPET thread. In this case, all 20 pages are "hot", so LPET will evict the first 5 pages (to reach low_threshold). After that, the new area will be mapped.

<u> Expected Memory Layout(without .text, .data etc. sections):

![image](https://user-images.githubusercontent.com/63795552/188273924-cfa11f03-8ccc-4283-9860-5a11d172b083.png)

When handling page-fault of area_5, we can see that DM-Handler wakes up LPET, which in turn starts running, and shows the current state of LSPT (data structure of the pages in  the instance). Note that all pages are marked as 'hot'.
```
--------------START HADNLING PAGE FAULT--------------
[Sat Sep 03 2022 08:12:39.860]:[Info]-[DmHandler] - UFFD_EVENT_PAGEFAULT in address = 0x1e20000
[Sat Sep 03 2022 08:12:39.860]:[Info]-[DmHandler] - reached high threshold, waking up lpet
[Sat Sep 03 2022 08:12:39.871]:[Info]-
-----------------START LPET-----------------
[Sat Sep 03 2022 08:12:39.871]:[Info]-[Lpet] - start address: 0x1d4c000
[Sat Sep 03 2022 08:12:39.871]:[Info]-page list state:
-----START LSPT ----
address : 0x1d4c000 pfn : 354924 is_idle: 0
address : 0x1d4d000 pfn : 354925 is_idle: 0
address : 0x1d4e000 pfn : 655824 is_idle: 0
address : 0x1d4f000 pfn : 193034 is_idle: 0
address : 0x1d50000 pfn : 72204 is_idle: 0
address : 0x1d51000 pfn : 508200 is_idle: 0
address : 0x1d52000 pfn : 183981 is_idle: 0
address : 0x1d53000 pfn : 437520 is_idle: 0
address : 0x1d54000 pfn : 15579 is_idle: 0
address : 0x1d55000 pfn : 659837 is_idle: 0
address : 0x1e14000 pfn : 374446 is_idle: 0
address : 0x1e15000 pfn : 566045 is_idle: 0
address : 0x1e16000 pfn : 306787 is_idle: 0
address : 0x1e17000 pfn : 414088 is_idle: 0
address : 0x1e18000 pfn : 94980 is_idle: 0
address : 0x1e19000 pfn : 163802 is_idle: 0
address : 0x1e1a000 pfn : 443503 is_idle: 0
address : 0x1e1b000 pfn : 457389 is_idle: 0
address : 0x1e1c000 pfn : 456716 is_idle: 0
address : 0x1e1d000 pfn : 641177 is_idle: 0
-----END LSPT ----
```
As all pages in instance are marked as 'hot', LPET evicts, as expected, the first 5 pages brought to the instance, which are basically area_1. Right after this it maps the requested pages, and finally DMS state is as follows:
```
-----START SPT ----
address : 0x1d4c000 location : DISK
address : 0x1d4d000 location : DISK
address : 0x1d4e000 location : DISK
address : 0x1d4f000 location : DISK
address : 0x1d50000 location : DISK
address : 0x1d51000 location : INSTANCE_0
address : 0x1d52000 location : INSTANCE_0
address : 0x1d53000 location : INSTANCE_0
address : 0x1d54000 location : INSTANCE_0
address : 0x1d55000 location : INSTANCE_0
address : 0x1e14000 location : INSTANCE_0
address : 0x1e15000 location : INSTANCE_0
address : 0x1e16000 location : INSTANCE_0
address : 0x1e17000 location : INSTANCE_0
address : 0x1e18000 location : INSTANCE_0
address : 0x1e19000 location : INSTANCE_0
address : 0x1e1a000 location : INSTANCE_0
address : 0x1e1b000 location : INSTANCE_0
address : 0x1e1c000 location : INSTANCE_0
address : 0x1e1d000 location : INSTANCE_0
address : 0x1e20000 location : INSTANCE_0
address : 0x1e21000 location : INSTANCE_0
address : 0x1e22000 location : INSTANCE_0
address : 0x1e23000 location : INSTANCE_0
address : 0x1e24000 location : INSTANCE_0
------END SPT-----
```

**Step 3:**

The code touches area_2, area_3 and marks them as 'hot'.
Code then accesses the first five pages of area 1 (that were evicted recently). This will end up with five page-faults, resolved by fetching them from disc. Because 20 pages are already in instance, LPET wakes up and looks for pages to evict, starting from area_2 (starting in 0x1d51000). Since area_2 and area_3 are 'hot' it passes over them and evicts all 5 pages in area_4.

<u> Expected memory layout:

![image](https://user-images.githubusercontent.com/63795552/188273614-bb6f7401-b549-4bc7-95ae-0f91134a023a.png)

In this part of the output, we see that LPET wakes up, and looks for the first idle page starting from 0x1d51000.
Note that area_2 (0x1d51000  - 0x1d55000 ) and area_3 (0x1e14000  - 0x1e18000 ) are marked 'hot' since user-code touched it just before LPET woke up, and area_5 (0x1e20000 - 0x1e24000) was accessed just before that.

    [Sat Sep 03 2022 08:12:40.063]:[Info]-    
    --------------START HADNLING PAGE FAULT--------------    
    [Sat Sep 03 2022 08:12:40.063]:[Info]-[DmHandler] - UFFD_EVENT_PAGEFAULT in address = 0x1d4c000    
    [Sat Sep 03 2022 08:12:40.064]:[Info]-[DmHandler] - reached high threshold, waking up lpet    
    [Sat Sep 03 2022 08:12:40.073]:[Info]-    
    -----------------START LPET-----------------    
    [Sat Sep 03 2022 08:12:40.074]:[Info]-[Lpet] - start address: 0x1d51000    
    [Sat Sep 03 2022 08:12:40.074]:[Info]-page list state:    
    -----START LSPT ----    
    address : 0x1d51000 pfn : 508200 is_idle: 0    
    address : 0x1d52000 pfn : 183981 is_idle: 0    
    address : 0x1d53000 pfn : 437520 is_idle: 0    
    address : 0x1d54000 pfn : 15579 is_idle: 0    
    address : 0x1d55000 pfn : 659837 is_idle: 0    
    address : 0x1e14000 pfn : 374446 is_idle: 0    
    address : 0x1e15000 pfn : 566045 is_idle: 0    
    address : 0x1e16000 pfn : 306787 is_idle: 0    
    address : 0x1e17000 pfn : 414088 is_idle: 0    
    address : 0x1e18000 pfn : 94980 is_idle: 0    
    address : 0x1e19000 pfn : 163802 is_idle: 1    
    address : 0x1e1a000 pfn : 443503 is_idle: 1    
    address : 0x1e1b000 pfn : 457389 is_idle: 1    
    address : 0x1e1c000 pfn : 456716 is_idle: 1    
    address : 0x1e1d000 pfn : 641177 is_idle: 1    
    address : 0x1e20000 pfn : 1165742 is_idle: 0    
    address : 0x1e21000 pfn : 1156436 is_idle: 0    
    address : 0x1e22000 pfn : 1173609 is_idle: 0    
    address : 0x1e23000 pfn : 1161715 is_idle: 0    
    address : 0x1e24000 pfn : 1051470 is_idle: 0    
    -----END LSPT ----

Finally DMS state is as follows:

    -----START SPT ----    
    address : 0x1d4c000 location : INSTANCE_0    
    address : 0x1d4d000 location : DISK    
    address : 0x1d4e000 location : DISK    
    address : 0x1d4f000 location : DISK    
    address : 0x1d50000 location : DISK    
    address : 0x1d51000 location : INSTANCE_0    
    address : 0x1d52000 location : INSTANCE_0    
    address : 0x1d53000 location : INSTANCE_0    
    address : 0x1d54000 location : INSTANCE_0    
    address : 0x1d55000 location : INSTANCE_0    
    address : 0x1e14000 location : INSTANCE_0    
    address : 0x1e15000 location : INSTANCE_0    
    address : 0x1e16000 location : INSTANCE_0    
    address : 0x1e17000 location : INSTANCE_0    
    address : 0x1e18000 location : INSTANCE_0    
    address : 0x1e19000 location : DISK    
    address : 0x1e1a000 location : DISK    
    address : 0x1e1b000 location : DISK    
    address : 0x1e1c000 location : DISK    
    address : 0x1e1d000 location : DISK    
    address : 0x1e20000 location : INSTANCE_0    
    address : 0x1e21000 location : INSTANCE_0    
    address : 0x1e22000 location : INSTANCE_0    
    address : 0x1e23000 location : INSTANCE_0    
    address : 0x1e24000 location : INSTANCE_0    
    ------END SPT-----

As we can see, cold pages were evicted to disc, while hot ones are still present in instance.