#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tcpserver.h"

int tcp_write(tcp_server_t *tcps, char *buf, int len)
{
    return write(tcps->connfd, buf, len);
}

/* 
return value:
    > 0: number of bytes read
    = 0: connection closed
    < 0: error
 */
int tcp_read(tcp_server_t *tcps, char *buf, int len)
{
    int ret = 0;
    ret = read(tcps->connfd, buf, len);
    if (ret > 0 && tcps->handler) {
        tcps->handler(tcps);
    }
    return ret;
}

/* 
return value:
    TIMEOUT_RET: poll error
    > 0: number of bytes read
    = 0: connection closed
    < 0: error
 */
int tcp_read_timeout(tcp_server_t *tcps, char *buf, int len, int ms)
{
    int ret = 0;
    struct pollfd fds[1];
    fds[0].fd = tcps->connfd;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, ms);
    if (ret > 0) {
        if (fds[0].revents & POLLIN) {
            ret = read(tcps->connfd, buf, len);
            if (ret > 0 && tcps->handler) {
                tcps->handler(tcps);
            }
        }
    }
    else if(ret == 0) { 
        return TIMEOUT_RET;  // poll timeout
    }
    return ret;
}

int tcp_wait_for_client(tcp_server_t *tcps, int timeout)
{
    int ret = 0;
    fd_set readfds;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (timeout == 0) {
        timeout = INT32_MAX;
    }
    struct timeval tv = {timeout, 0};

    FD_ZERO(&readfds);
    FD_SET(tcps->sockfd, &readfds);

    int sel = select(tcps->sockfd + 1, &readfds, NULL, NULL, &tv);
    if (sel > 0 && FD_ISSET(tcps->sockfd, &readfds)) {
        tcps->connfd = accept(tcps->sockfd, (struct sockaddr *)&addr, &addrlen);
    }
    else {
        return -1;
    }
    return ret;
}

void tcp_set_client_handler(tcp_server_t *tcps, client_handler handler)
{
    tcps->handler = handler;
}

int tcp_start_server(tcp_server_t *tcps, const char *addr, int port)
{
    int ret = 0;
    tcps->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcps->sockfd < 0) {
        perror("socket");
        return -1;
    }
    memset(&tcps->myaddr, 0, sizeof(tcps->myaddr));
    tcps->myaddr.sin_family = AF_INET;
    tcps->myaddr.sin_port = htons(port);
    tcps->myaddr.sin_addr.s_addr = inet_addr(addr);

    ret = bind(tcps->sockfd, (struct sockaddr *)&tcps->myaddr, sizeof(tcps->myaddr));
    if (ret < 0) {
        perror("bind");
        return -1;
    }

    ret = listen(tcps->sockfd, 8);// 8 is the maximum number of pending connections
    if (ret < 0) {
        perror("listen");
        return -1;
    }

    return ret;
}

void tcp_stop_server(tcp_server_t *tcps)
{
    close(tcps->sockfd);// Close the server socket
    close(tcps->connfd);// Close the connection socket
}