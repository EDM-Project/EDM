#include "mpiDms.h"
#define NUM_OF_RETRIES 3
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

void MpiDms::SendAckForEvictPage(uintptr_t vaddr, MPI_EDM::request_evict_page_status status) {
    AckPage ack_page;
    ack_page.vaddr = vaddr;
    LOG(DEBUG) << "[DMS] request_evict_page_status is " << status;
    std::string error_message = "";
    if (status == fail) {
        error_message = "failed to evict page";
    }   
    memcpy(ack_page.error, error_message.c_str(), ERROR_SIZE);
    MPI_Datatype MPI_AckPage = GetMPIDataType(ack_page);
    MPI_Send(&ack_page, 1, MPI_AckPage, 0, ACK_TAG, mpi_comm);
}
}