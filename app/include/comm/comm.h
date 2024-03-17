#ifndef __COMM_H__
#define __COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "comm_platfrom.h"

typedef struct comm_op comm_op_t;

typedef struct comm {
    void *priv;
    int (*read_callback)(struct comm *comm, void *);
    const comm_op_t *op;
} comm_t;

int comm_read(comm_t *comm, void *value, int size);
int comm_write(comm_t *comm, void *value, int size);
int comm_export(comm_t *comm);
int comm_unexport(comm_t *comm);
int comm_set_callback(comm_t *comm, int (*callback)(struct comm *comm, void *));

comm_t *comm_create_with_register(enum COMM_TYPE type);
int comm_register(comm_t *comm, enum COMM_TYPE type);
void comm_destroy(comm_t *comm);

#ifdef __cplusplus
}
#endif

#endif