#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <netinet/in.h>
#include <stdbool.h>

#define TIMEOUT_RET 0xffffffff

typedef void (*client_handler)(void *tcps);

typedef struct tcp_server {
    int sockfd;
    int connfd;
    struct sockaddr_in myaddr;
    client_handler handler;
    bool connected;
} tcp_server_t;

int tcp_write(tcp_server_t *tcps, char *buf, int len);
int tcp_read(tcp_server_t *tcps, char *buf, int len);
int tcp_read_timeout(tcp_server_t *tcps, char *buf, int len, int ms);
int tcp_wait_for_client(tcp_server_t *tcps, int timeout);
void tcp_set_client_handler(tcp_server_t *tcps, client_handler handler);
bool tcp_is_connected(tcp_server_t *tcps);
int tcp_start_server(tcp_server_t *tcps, const char *addr, int port);
void tcp_stop_server(tcp_server_t *tcps);

#endif /* TCPSERVER_H */