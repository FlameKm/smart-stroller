#include "hal_gpio.h"
#include "log.h"
#include <unistd.h>

/* 
AI, hello, help me write gpio unity test.
    int gpio_set_direction(gpio_dev_t *gpio, GPIO_DIRECTION dir);
    int gpio_get_direction(gpio_dev_t *gpio);
    int gpio_get_value(gpio_dev_t *gpio);
    void gpio_set_value(gpio_dev_t *gpio, int value);
    gpio_dev_t *gpio_create(int port, GPIO_DIRECTION dir);
    void gpio_destroy(gpio_dev_t *gpio); 
*/

int main()
{
    gpio_dev_t *gpio74 = gpio_create(74, GPIO_DIRECTION_OUT);
    gpio_dev_t *gpio233 = gpio_create(233, GPIO_DIRECTION_IN);
    if(gpio74 == NULL || gpio233 == NULL)
    {
        log_error("gpio create failed");
        return -1;
    }

    int value;
    gpio_set_value(gpio74, 1);
    value = gpio_get_value(gpio233);
    log_info("gpio233 value = %d and gpio74 value set to 1", value);
    log_info( "pleass waiting 1s");
    sleep(1);

    gpio_set_value(gpio74, 0);
    value = gpio_get_value(gpio233);
    log_info("gpio233 value = %d and gpio74 value set to 0", value);

    gpio_destroy(gpio74);
    gpio_destroy(gpio233);
    return 0;
}