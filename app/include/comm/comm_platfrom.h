#ifndef __COMM_PLATFROM_H__
#define __COMM_PLATFROM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum COMM_TYPE
{
    COMM_TYPE_UART,
    COMM_TYPE_TCP,
} COMM_TYPE;

typedef struct comm comm_t;

int uart_comm_register(comm_t *comm);


#ifdef __cplusplus
}
#endif

#endif