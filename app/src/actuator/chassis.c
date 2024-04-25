#include "chassis.h"
#include <stdlib.h>

#define USR_OPEN_LOOP

#define USE_GPIO_IN1 73
#define USE_GPIO_IN2 72
#define USE_GPIO_IN3 70
#define USE_GPIO_IN4 69

#define MOTOR_ID_RIGHT 0
#define MOTOR_ID_LEFT 1

int set_chassis_speed(chassis_t *chassis, float speed)
{
    int ret = 0;
    uint32_t speed_i;
    if (speed > 0) {
        speed_i = (uint32_t)speed;
        gpio_set_value(chassis->gpio_lpositive, 1);
        gpio_set_value(chassis->gpio_lnegative, 0);
        gpio_set_value(chassis->gpio_rpositive, 1);
        gpio_set_value(chassis->gpio_rnegative, 0);
    }
    else {
        speed_i = (uint32_t)-speed;
        gpio_set_value(chassis->gpio_lpositive, 0);
        gpio_set_value(chassis->gpio_lnegative, 1);
        gpio_set_value(chassis->gpio_rpositive, 0);
        gpio_set_value(chassis->gpio_rnegative, 1);
    }
    ret = motor_set_speed(chassis->ml, speed_i);
    ret |= motor_set_speed(chassis->mr, speed_i);
    if (!ret) {
        chassis->speed = speed;
    }
    return ret;
}

int set_chassis_turn(chassis_t *chassis, float turn)
{
    int ret = 0;
    float angle;
    if (turn <= -40.0 || turn >= 40.0) {
        return -1;
    }
    angle = turn + 90.0;
    ret = servo_set_angle(chassis->servo, angle);
    if (!ret) {
        servo_start(chassis->servo);
        chassis->turn = turn;
    }
    return ret;
}

float get_chassis_speed(chassis_t *chassis)
{
#ifdef USR_OPEN_LOOP
    return chassis->speed;
#endif
    int ret = 0;
    uint32_t s1, s2;
    ret = motor_get_speed(chassis->ml, &s1);
    ret |= motor_get_speed(chassis->mr, &s2);
    if (ret < 0) {
        return 0;
    }
    return (s1 + s2) / 2.0f;
}

float get_chassis_turn(chassis_t *chassis)
{
#ifdef USR_OPEN_LOOP
    return chassis->turn;
#endif
    int ret = 0;
    float angle;
    ret = servo_get_angle(chassis->servo, &angle);
    if (ret) {
        return 0;
    }
    return angle - 90.0;
}

int chassis_register(chassis_t *chassis)
{
    chassis->mr = motor_create(MOTOR_CURRENT_OPEN, MOTOR_ID_RIGHT);
    if (chassis->mr == NULL) {
        return -1;
    }
    chassis->ml = motor_create(MOTOR_CURRENT_OPEN, MOTOR_ID_LEFT);
    if (chassis->ml == NULL) {
        motor_destroy(chassis->mr);
        return -2;
    }

    chassis->servo = servo_create();
    if (chassis->servo == NULL) {
        motor_destroy(chassis->mr);
        motor_destroy(chassis->ml);
        return -3;
    }

    chassis->gpio_lpositive = gpio_create(USE_GPIO_IN1, GPIO_DIRECTION_OUT);
    gpio_set_value(chassis->gpio_lpositive, 0);
    chassis->gpio_lnegative = gpio_create(USE_GPIO_IN2, GPIO_DIRECTION_OUT);
    gpio_set_value(chassis->gpio_lnegative, 0);
    chassis->gpio_rpositive = gpio_create(USE_GPIO_IN3, GPIO_DIRECTION_OUT);
    gpio_set_value(chassis->gpio_rpositive, 0);
    chassis->gpio_rnegative = gpio_create(USE_GPIO_IN4, GPIO_DIRECTION_OUT);
    gpio_set_value(chassis->gpio_rnegative, 0);
    return 0;
}
void chassis_destroy(chassis_t *chassis)
{
    gpio_destroy(chassis->gpio_lpositive);
    gpio_destroy(chassis->gpio_lnegative);
    gpio_destroy(chassis->gpio_rpositive);
    gpio_destroy(chassis->gpio_rnegative);
    
    motor_destroy(chassis->mr);
    motor_destroy(chassis->ml);
    servo_destroy(chassis->servo);
}