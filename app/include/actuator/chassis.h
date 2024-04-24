#ifndef __CHASSIS_H
#define __CHASSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "motor.h"
#include "servo.h"

typedef struct chassis {
    float speed;
    float turn;// direction
    motor_t *ml;
    motor_t *mr;
    servo_t *servo;
} chassis_t;


int set_chassic_speed(chassis_t *chassis, float speed);
int set_chassic_turn(chassis_t *chassis, float turn);
float get_chassic_speed(chassis_t *chassis);
float get_chassic_turn(chassis_t *chassis);
int chassis_register(chassis_t *chassis);
void chassis_destroy(chassis_t *chassis);

#ifdef __cplusplus
}
#endif

#endif// __CHASSIS_H