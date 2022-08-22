
#include "MpiEdm.h"
#include "CPPLogger.h"
namespace MPI_EDM {

MpiDms::MpiDms (int argc, char *argv[])  {
    MPI_Init(&argc, &argv);
    // Open port.
    //char port_name[MPI_MAX_PORT_NAME];
    MPI_Open_port(MPI_INFO_NULL, port_name);
    // Publish port name and accept client.
    MPI_Publish_name("dms", MPI_INFO_NULL, port_name);
    MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &mpi_comm);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}
RequestGetPageData MpiDms::ListenRequestGetPage () {

    RequestGetPageData request;
    MPI_Datatype requestPage = GetMPIDataType(request);
    MPI_Recv(&request, 1, requestPage, 0, PAGE_REQUEST_TAG, mpi_comm, MPI_STATUS_IGNORE);
    return request;

}
void MpiDms::SendPageBackToApp (RequestGetPageData response) {
    
    MPI_Datatype requestPage = GetMPIDataType(response);
    MPI_Send(&response, 1, requestPage, 0, PAGE_REQUEST_TAG, mpi_comm);

}

RequestEvictPageData MpiDms::ListenRequestEvictPage() {
    RequestEvictPageData request;
    MPI_Datatype MPI_RequestEvictPage = GetMPIDataType(request);
    MPI_Recv(&request, 1, MPI_RequestEvictPage, 0, EVICT_REQUEST_TAG, mpi_comm, MPI_STATUS_IGNORE);
    return request;
}

void MpiDms::SendAckForEvictPage(uintptr_t vaddr) {
    AckPage ack_page;
    ack_page.vaddr = vaddr;
    memcpy(ack_page.error, "NONE", ERROR_SIZE);
    MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);
    MPI_Send(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, mpi_comm);
}

//mpi app

MpiApp::MpiApp(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    // Look up for server's port name.
    char port_name[MPI_MAX_PORT_NAME];
    MPI_Lookup_name("dms", MPI_INFO_NULL, port_name);
    // Connect to server.
    MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &mpi_comm);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

RequestGetPageData MpiApp::RequestPageFromDMS (uintptr_t vaddr) {
    RequestGetPageData request_page;
    request_page.vaddr = vaddr;
    MPI_Datatype MPI_RequestPage = GetMPIDataType(request_page);
    MPI_Send(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, mpi_comm);
    LOG(DEBUG) << "[DM HANDLER] - request the page in address " << PRINT_AS_HEX(vaddr) ;
    MPI_Recv(&request_page, 1, MPI_RequestPage, 0, PAGE_REQUEST_TAG, mpi_comm, MPI_STATUS_IGNORE);
    LOG(DEBUG) << "[DM HANDLER] - get response of the page in address " << PRINT_AS_HEX(vaddr) ;
    // TODO: Error handling 

    return request_page;
}
std::string MpiApp::RequestEvictPage (uintptr_t vaddr, char* page) {
    
    RequestEvictPageData request;
    memcpy(request.page, page, PAGE_SIZE);
    request.vaddr = vaddr;

    MPI_Datatype MPI_RequestEvictPage = GetMPIDataType(request);
    AckPage ack_page;
    MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);


    MPI_Send(&request, 1, MPI_RequestEvictPage, 0, EVICT_REQUEST_TAG, mpi_comm);
    LOG(DEBUG) << "[DM HANDLER] - send requet to evict page in address " << PRINT_AS_HEX(request.vaddr) ;
    MPI_Recv(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, mpi_comm, MPI_STATUS_IGNORE);
    LOG(DEBUG) << "[DM HANDLER] - got ack for evict page in " << PRINT_AS_HEX(ack_page.vaddr) ;

    //TODO: ERROR HANDLING AND RETURN ERROR
    std::string error_str (ack_page.error, ack_page.error + ERROR_SIZE);

    return error_str;

}

}