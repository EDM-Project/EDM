#include "mpi.h"
#include <stdio.h>
#include <sys/mman.h>
#include "MpiStructs.h"
#include <unistd.h>

#define PAGE_REQUEST_TAG 0
#define EVICT_REQUEST_TAG 1
#define ACK_TAG 2

namespace MPI_EDM {
class MpiEdm {
private:

public:
    int rank;
    MPI_Comm CommApp;
    char port_name[MPI_MAX_PORT_NAME];

};

class MpiDms : public MpiEdm {
private:
    
public:
    
    MpiDms (int argc, char *argv[])  {
        MPI_Init(&argc, &argv);
        // Open port.
        //char port_name[MPI_MAX_PORT_NAME];
        MPI_Open_port(MPI_INFO_NULL, port_name);
        // Publish port name and accept client.
        MPI_Publish_name("dms", MPI_INFO_NULL, port_name);
        MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &CommApp);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }
    RequestGetPageData ListenRequestPage () {

        RequestGetPageData received;
        MPI_Datatype requestPage = GetMPIDataType(received);
        MPI_Recv(&received, 1, requestPage, 0, PAGE_REQUEST_TAG, CommApp, MPI_STATUS_IGNORE);
        return received;

    }
    void HandleRequestGetPage (RequestGetPageData received) {
        // read from disk , meanwhile simple page
        char* mem  = (char*) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        for (int i =0; i< PAGE_SIZE ; i++) {
            mem[i] = 'b';
        }
        RequestGetPageData response;
        MPI_Datatype requestPage = GetMPIDataType(response);
        response.vaddr = received.vaddr;
        memcpy ( response.page, mem, PAGE_SIZE );
        memcpy ( response.flags, received.flags, strlen(received.flags) +1 );
        std::cout << "DMS - i got your request , now send response" << std::endl; 
        MPI_Send(&response, 1, requestPage, 0, PAGE_REQUEST_TAG, CommApp);

    }
    void HandleRequestEvictPage () {

        RequestEvictPageData request;
        MPI_Datatype MPI_RequestEvictPage = GetMPIDataType(request);
        std::cout << "DMS - waiting to evict pages" << std::endl; 
        MPI_Recv(&request, 1, MPI_RequestEvictPage, 0, EVICT_REQUEST_TAG, CommApp, MPI_STATUS_IGNORE);
        // send page to disk
        AckPage ack_page;
        ack_page.vaddr = request.vaddr;
        memcpy(ack_page.error, "NONE", ERROR_SIZE);
        MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);
        std::cout << "DMS - page evicted, sending ack" << std::endl; 
        MPI_Send(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, CommApp);

    }
    

};

class MpiApp : public MpiEdm {

private:

public:

    MpiApp(int argc, char *argv[]) {
        MPI_Init(&argc, &argv);
        // Look up for server's port name.
        char port_name[MPI_MAX_PORT_NAME];
        MPI_Lookup_name("dms", MPI_INFO_NULL, port_name);
        // Connect to server.
        MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &CommApp);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }

    RequestGetPageData RequestPageFromDMS (uintptr_t vaddr) {
        RequestGetPageData request_page;
        request_page.vaddr = vaddr;
        MPI_Datatype MPI_RequestPage = GetMPIDataType(request_page);
        MPI_Send(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, CommApp);
        std::cout << "DM- requet page in address 0x" << vaddr << std::endl;
        MPI_Recv(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, CommApp, MPI_STATUS_IGNORE);
        std::cout << "DM- get response of page in address 0x" << vaddr << std::endl;
        // TODO: Error handling 

        return request_page;
    }
    std::string RequestEvictPage (uintptr_t vaddr, char* page) {
        
        RequestEvictPageData request;
        memcpy(request.page, page, PAGE_SIZE);
        request.vaddr = vaddr;

        MPI_Datatype MPI_RequestEvictPage = GetMPIDataType(request);
        AckPage ack_page;
        MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);

    
        MPI_Send(&request, 1, MPI_RequestEvictPage, 0, EVICT_REQUEST_TAG, CommApp);
        std::cout << "DM- requet evict page in address 0x" << request.vaddr << std::endl;
        MPI_Recv(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, CommApp, MPI_STATUS_IGNORE);
        std::cout << "DM- got ack for evict page in 0x" << ack_page.vaddr << std::endl;

        //TODO: ERROR HANDLING AND RETURN ERROR
        std::string error_str (ack_page.error, ack_page.error + ERROR_SIZE);

        return error_str;

    }
    

};
}
