#ifndef __COMM_OP_H__
#define __COMM_OP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "comm.h"

typedef struct comm_op {
    int (*init)(comm_t *comm);
    int (*deinit)(comm_t *comm);
    int (*write)(comm_t *comm, void *value, int size);
    int (*read)(comm_t *comm, void *value, int size);
} comm_op_t;


#ifdef __cplusplus
}
#endif

#endif