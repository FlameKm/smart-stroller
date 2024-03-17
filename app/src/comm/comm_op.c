#include "comm_op.h"

int comm_read(comm_t *comm, void *value, int size)
{
    if (comm->op->read) {
        return comm->op->read(comm, value, size);
    }
    else {
        return -1;
    }
}

int comm_write(comm_t *comm, void *value, int size)
{
    if (comm->op->read) {
        return comm->op->read(comm, value, size);
    }
    else {
        return -1;
    }
}

int comm_export(comm_t *comm)
{
    if (comm->op->init) {
        return comm->op->init(comm);
    }
    else {
        return -1;
    }
}

int comm_unexport(comm_t *comm)
{
    if (comm->op->deinit) {
        return comm->op->deinit(comm);
    }
    else {
        return -1;
    }
}

int comm_set_callback(comm_t *comm, int (*callback)(struct comm *comm, void *))
{
    comm->read_callback = callback;
    return 0;
}
