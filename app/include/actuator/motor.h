#ifndef __MOTOR_H__
#define __MOTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum MOTOR_WORK_TYPE
{
    MOTOR_CURRENT_OPEN,
    MOTOR_SPEED_RING,
    MOTOR_ANGLE_RING,
} MOTOR_WORK_TYPE;

typedef struct motor motor_t;

int motor_set_speed(motor_t *motor, uint32_t speed);
int motor_get_speed(motor_t *motor, uint32_t *speed);
int motor_set_angle(motor_t *motor, uint32_t angle);
int motor_get_angle(motor_t *motor, uint32_t *angle);
motor_t *motor_create(enum MOTOR_WORK_TYPE, int id);
void motor_destroy(motor_t *motor);

#ifdef __cplusplus
}
#endif

#endif