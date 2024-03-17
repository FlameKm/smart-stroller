//
// Created by hyc on 24-2-4.
//
#include <signal.h>
#include <stdio.h>
#include "stroller.h"
#include <stdbool.h>
#include <unistd.h>
#include "log.h"

bool contine = true;

void stop(int signo)
{
    log_error("oops! stop!!!");
    contine = false;
}

int main()
{
    int ret = 0;
    stroller_t *strl;

    strl = strl_create();
    if (strl == NULL) {
        return -1;
    }
    log_info("stroller init ok");

    signal(SIGINT, stop);
    ret = strl_start_loop(strl);
    if (ret) {
        contine = false;
    }
    log_info("stroller start loop");

    while (contine)
        pause();

    strl_destroy(strl);
    log_info("stroller destroy");
}