#include "chassis.h"
#include <stdlib.h>

#define USR_OPEN_LOOP

#define MOTOR_ID_RIGHT 0
#define MOTOR_ID_LEFT 1

int set_chassic_speed(chassis_t *chassis, float speed)
{
    int ret = 0;
    int speed_i = (int)speed;
    ret = motor_set_speed(chassis->ml, speed_i);
    ret |= motor_set_speed(chassis->mr, speed_i);
    if (!ret) {
        chassis->speed = speed;
    }
    return ret;
}

int set_chassic_turn(chassis_t *chassis, float turn)
{
    int ret = 0;
    float angle;
    if (turn <= -40.0 || turn >= 40.0) {
        return -1;
    }
    angle = turn + 90.0;
    ret = servo_set_angle(chassis->servo, angle);
    if (!ret) {
        chassis->turn = turn;
    }
    return ret;
}

float get_chassic_speed(chassis_t *chassis)
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

float get_chassic_turn(chassis_t *chassis)
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

    return 0;
}
void chassis_destroy(chassis_t *chassis)
{
    motor_destroy(chassis->mr);
    motor_destroy(chassis->ml);
    servo_destroy(chassis->servo);
}