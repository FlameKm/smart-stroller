#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <pthread.h>

typedef enum GPIO_DIRECTION{
    GPIO_DIRECTION_IN,
    GPIO_DIRECTION_OUT,
}GPIO_DIRECTION;

typedef enum GPIO_LEVEL{
    GPIO_LEVEL_LOW,
    GPIO_LEVEL_HIGH,
}GPIO_LEVEL;

typedef struct gpio_dev {
    int port;
    int value;
    GPIO_DIRECTION dir;
    pthread_mutex_t lock;
} gpio_dev_t;

int gpio_set_direction(gpio_dev_t *gpio, GPIO_DIRECTION dir);
int gpio_get_direction(gpio_dev_t *gpio);
int gpio_get_value(gpio_dev_t *gpio);
void gpio_set_value(gpio_dev_t *gpio, int value);
gpio_dev_t *gpio_create(int port, GPIO_DIRECTION dir);
void gpio_destroy(gpio_dev_t *gpio);

#endif