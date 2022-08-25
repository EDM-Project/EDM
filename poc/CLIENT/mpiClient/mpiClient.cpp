#include "mpiClient.h"
#define  NUM_OF_RETRIES 3
namespace MPI_EDM{
MpiClient::MpiClient(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    // Look up for server's port name.
    char port_name[MPI_MAX_PORT_NAME];
    MPI_Lookup_name("dms", MPI_INFO_NULL, port_name);
    // Connect to server.
    MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &mpi_comm);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

RequestGetPageData MpiClient::RequestPageFromDMS (uintptr_t vaddr) {
    RequestGetPageData request_page;
    request_page.vaddr = vaddr;
    MPI_Datatype MPI_RequestPage = GetMPIDataType(request_page);
    
    for (int i =0 ; i < NUM_OF_RETRIES ; i++)
    {
        MPI_Send(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, mpi_comm);
        LOG(DEBUG) << "[DM HANDLER] - request the page in address " << PRINT_AS_HEX(vaddr) ;
        MPI_Recv(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, mpi_comm, MPI_STATUS_IGNORE);
        LOG(DEBUG) << "[DM HANDLER] - get response of the page in address " << PRINT_AS_HEX(vaddr) ;
        if (request_page.info == MPI_EDM::error){
            LOG(ERROR) << "[Uffd] - RequestPageFromDMS failed. Retry " ;
        }
        else{
            LOG(DEBUG) << "[Uffd] - RequestPageFromDMS succeeded. " ;
            break;
        }
    }
    return request_page;
}
void MpiClient::RequestEvictPage (uintptr_t vaddr, char* page) {
    
    RequestEvictPageData request;
    memcpy(request.page, page, PAGE_SIZE);
    request.vaddr = vaddr;

    MPI_Datatype MPI_RequestEvictPage = GetMPIDataType(request);
    AckPage ack_page;
    MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);


    for (int i= 0 ; i < NUM_OF_RETRIES ; i++) {

        MPI_Send(&request, 1, MPI_RequestEvictPage, 0, EVICT_REQUEST_TAG, mpi_comm);
        LOG(DEBUG) << "[DM HANDLER] - send requet to evict page in address " << PRINT_AS_HEX(request.vaddr) ;
        MPI_Recv(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, mpi_comm, MPI_STATUS_IGNORE);
        LOG(DEBUG) << "[DM HANDLER] - got ack for evict page in " << PRINT_AS_HEX(ack_page.vaddr) << 
            "Ack message : " << ack_page.error;

        if (std::string(ack_page.error).empty()) {
            break;
        }
    }

}
}