#include "mpiStructs.h"

namespace MPI_EDM {

MPI_Datatype GetMPIDataType(RequestGetPageData request_page) {
    MPI_Datatype MPI_RequestPage;
    int lengths[3] = { 8, PAGE_SIZE, 4 };
 
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    MPI_Get_address(&request_page, &base_address);
    MPI_Get_address(&request_page.vaddr, &displacements[0]);
    MPI_Get_address(&request_page.page[0], &displacements[1]);
    MPI_Get_address(&request_page.info, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
 
    MPI_Datatype types[3] = { MPI_UNSIGNED_LONG_LONG, MPI_CHAR, MPI_INT32_T };
    MPI_Type_create_struct(3, lengths, displacements, types, &MPI_RequestPage); 
    MPI_Type_commit(&MPI_RequestPage);
    return MPI_RequestPage;

}

MPI_Datatype GetMPIDataType(RequestEvictPageData request) {
    MPI_Datatype MPI_RequestEvictPage;
    int lengths[3] = { 8, PAGE_SIZE, 4};
 
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    MPI_Get_address(&request, &base_address);
    MPI_Get_address(&request.vaddr, &displacements[0]);
    MPI_Get_address(&request.page[0], &displacements[1]);
    MPI_Get_address(&request.info, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);

    MPI_Datatype types[3] = { MPI_UNSIGNED_LONG_LONG, MPI_CHAR, MPI_INT32_T};
    MPI_Type_create_struct(3, lengths, displacements, types, &MPI_RequestEvictPage); 
    MPI_Type_commit(&MPI_RequestEvictPage);
    return MPI_RequestEvictPage;

}

MPI_Datatype GetMPIDataType(AckPage ack) {
    MPI_Datatype MPI_AckPage;
    int lengths[2] = { 8, ERROR_SIZE};
 
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    MPI_Get_address(&ack, &base_address);
    MPI_Get_address(&ack.vaddr, &displacements[0]);
    MPI_Get_address(&ack.error[0], &displacements[1]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
 
    MPI_Datatype types[2] = { MPI_UNSIGNED_LONG_LONG, MPI_CHAR};
    MPI_Type_create_struct(2, lengths, displacements, types, &MPI_AckPage); 
    MPI_Type_commit(&MPI_AckPage);
    return MPI_AckPage;

}

}