#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <tcpserver.h>
#include "log.h"

bool is_stop = false;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        log_info("received SIGINT");
        is_stop = true;
    }
}

char rbuf[100];
void handle_client(void *tcps)
{
    // 返回原数据和加密过的数据
    printf("rec: %s\n", rbuf);
    tcp_write(tcps, rbuf, sizeof(rbuf));//echo back
    tcp_write(tcps, ": ", 2);
    char sendbuf[100];
    for (int i = 0; i < sizeof(rbuf); i++) {
        sendbuf[i] = rbuf[i] + 1;
    }
    tcp_write(tcps, sendbuf, sizeof(sendbuf));
}

void *client_thread(void *arg)
{
    tcp_server_t *tcps = (tcp_server_t *)arg;
    int ret = 0;
loop:
    ret = tcp_wait_for_client(tcps, 0);
    if (ret < 0) {
        perror("accept");
        return NULL;
    }
    log_debug("accept client connect");
    while (!is_stop) {
        memset(rbuf, 0, sizeof(rbuf));
        ret = tcp_read(tcps, rbuf, sizeof(rbuf));
        if (ret == 0) {
            log_debug("Close connection from client");
            goto loop;
        }
        else if (ret < 0) {
            return NULL;
        }
    }
    return NULL;
}

int main()
{
    tcp_server_t server;
    int port = 8888;
    int ret;

    signal(SIGINT, sig_handler);

    ret = tcp_start_server(&server, "0.0.0.0", port);
    if (ret != 0) {
        log_error("Failed to start server on port %d", port);
        return 1;
    }
    log_info("Server started on port %d", port);
    tcp_set_client_handler(&server, handle_client);

    pthread_t clientThread;
    pthread_create(&clientThread, NULL, client_thread, &server);

    while (!is_stop) {
        sleep(1);
    }
    pthread_cancel(clientThread);
    tcp_stop_server(&server);
    return 0;
}