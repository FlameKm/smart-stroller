#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "hal_gpio.h"

int gpio_set_direction(gpio_dev_t *gpio, GPIO_DIRECTION dir)
{
    char path[35];
    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio->port);
    pthread_mutex_lock(&gpio->lock);
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        goto err;
    }
    if (dir == GPIO_DIRECTION_IN) {
        write(fd, "in", 2);
    }
    else {
        write(fd, "out", 3);
    }
    close(fd);
    pthread_mutex_unlock(&gpio->lock);
    gpio->dir = dir;
    return 0;
err:
    return -1;
}

int gpio_get_direction(gpio_dev_t *gpio)
{
    char path[35];
    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio->port);
    pthread_mutex_lock(&gpio->lock);
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        goto err;
    }
    char dir[4];
    read(fd, dir, 3);
    close(fd);
    pthread_mutex_unlock(&gpio->lock);
    if (strncmp(dir, "in", 2) == 0) {
        gpio->dir = GPIO_DIRECTION_IN;
    }
    else {
        gpio->dir = GPIO_DIRECTION_OUT;
    }
    return gpio->dir;
err:
    return -1;
}

int gpio_get_value(gpio_dev_t *gpio)
{
    char path[35];
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio->port);
    pthread_mutex_lock(&gpio->lock);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        goto err;
    }
    char value[2];
    read(fd, value, 1);
    close(fd);
    pthread_mutex_unlock(&gpio->lock);
    gpio->value = atoi(value);
    return gpio->value;
err:
    return -1;
}

void gpio_set_value(gpio_dev_t *gpio, int value)
{
    char path[35];
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio->port);
    pthread_mutex_lock(&gpio->lock);
    int fd = open(path, O_WRONLY);
    if (fd != -1) {
        char val_str[2];
        sprintf(val_str, "%d", value);
        write(fd, val_str, 1);
        close(fd);
    }
    pthread_mutex_unlock(&gpio->lock);
    gpio->value = value;
}

gpio_dev_t *gpio_create(int port, GPIO_DIRECTION dir)
{
    gpio_dev_t *gpio = calloc(1, sizeof(gpio_dev_t));
    if (!gpio) {
        return NULL;
    }
    char cmd[50];
    sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
    system(cmd);

    gpio->port = port;
    gpio_set_direction(gpio, dir);
    return gpio;
}

void gpio_destroy(gpio_dev_t *gpio)
{
    if (!gpio) {
        return;
    }
    char cmd[50];
    sprintf(cmd, "echo %d > /sys/class/gpio/unexport", gpio->port);
    system(cmd);
    free(gpio);
}
