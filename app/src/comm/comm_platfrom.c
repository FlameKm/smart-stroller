#include "comm_op.h"
#include <stdlib.h>


int comm_register(comm_t *comm, enum COMM_TYPE type)
{
    int ret = 0;
    switch (type) {
        case COMM_TYPE_UART:
            ret = uart_comm_register(comm);
            break;
        default:
            ret = -1;
            break;
    }
    if (ret) {
        comm->op->init(comm);
    }
    return ret;
}

comm_t *comm_create_with_register(enum COMM_TYPE type)
{
    comm_t *comm;
    int ret = 0;
    comm = calloc(1, sizeof(comm_t));
    if (comm == NULL) {
        return NULL;
    }
    ret = comm_register(comm, type);
    if (ret < 0) {
        free(comm);
        return NULL;
    }
    return comm;
}

void comm_destroy(comm_t *comm)
{
    if (comm == NULL) {
        return;
    }
    comm->op->deinit(comm);
    free(comm);
}