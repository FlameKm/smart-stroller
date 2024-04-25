#ifndef __CHASSIS_H
#define __CHASSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_gpio.h"
#include "motor.h"
#include "servo.h"

typedef struct chassis {
    float speed;
    float turn;// direction
    motor_t *ml;
    gpio_dev_t *gpio_lpositive;
    gpio_dev_t *gpio_lnegative;
    motor_t *mr;
    gpio_dev_t *gpio_rpositive;
    gpio_dev_t *gpio_rnegative;
    servo_t *servo;

} chassis_t;


int set_chassis_speed(chassis_t *chassis, float speed);
int set_chassis_turn(chassis_t *chassis, float turn);
float get_chassis_speed(chassis_t *chassis);
float get_chassis_turn(chassis_t *chassis);
int chassis_register(chassis_t *chassis);
void chassis_destroy(chassis_t *chassis);

#ifdef __cplusplus
}
#endif

#endif// __CHASSIS_H