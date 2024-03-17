#include <stdlib.h>
#include "comm_op.h"

typedef struct uart_data {
    void *null_data;
} uart_data_t;

static int uart_init(comm_t *comm)
{
    comm->priv = calloc(1, sizeof(uart_data_t));
    return 0;
}

static int uart_deinit(comm_t *comm)
{
    free(comm->priv);
    return 0;
}

static int uart_read(comm_t *comm, void *value, int size)
{
    // todo
    return 0;
}

static int uart_write(comm_t *comm, void *value, int size)
{
    // todo
    return 0;
}

static const comm_op_t uart_op = {
    .init = uart_init,
    .deinit = uart_deinit,
    .read = uart_read,
    .write = uart_write,
};

int uart_comm_register(comm_t *comm)
{
    comm->op = &uart_op;
    return 0;
}