#include <stdlib.h>
#include "motor.h"

typedef struct motor {
    void *temp;
} motor_t;

// todo


motor_t *motor_create(enum MOTOR_WORK_TYPE type)
{
    motor_t *motor = malloc(sizeof(motor_t));
    return motor;
}

void motor_destroy(motor_t *motor)
{
}