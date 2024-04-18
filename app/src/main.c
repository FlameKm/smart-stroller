//
// Created by hyc on 24-2-4.
//
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "log.h"
#include "stroller.h"

#define LOG_LEVER LOG_DEBUG

bool contine = true;

static void stop(int signo)
{
    log_error("oops! stop!!!");
    contine = false;
}

static pthread_mutex_t lock;
static void log_lock_function(bool lock, void *udata)
{
    if (lock) {
        pthread_mutex_lock((pthread_mutex_t *)udata);
    }
    else {
        pthread_mutex_unlock((pthread_mutex_t *)udata);
    }
}

int main()
{
    int ret = 0;
    stroller_t *strl;

    log_set_level(LOG_LEVER);
    /* Log supports concurrency */
    log_set_lock(log_lock_function, &lock);

    strl = stlr_create();
    if (strl == NULL) {
        return -1;
    }
    log_info("stroller init ok");

    signal(SIGINT, stop);
    ret = stlr_start(strl);
    if (ret) {
        contine = false;
    }
    log_info("stroller start loop");

    while (contine)
        pause();

    stlr_stop();
    stlr_destroy(strl);
    log_info("stroller destroy");
}