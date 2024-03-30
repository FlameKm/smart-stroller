#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "motor.h"
#include "log.h"

#define MOTOR_SET_SPEED 0// uint32_t
#define MOTOR_GET_SPEED 1
#define MOTOR_START 2
#define MOTOR_STOP 3

typedef struct motor {
    int fd;
    int speed;
    int speed_max;
    int speed_min;
} motor_t;

int motor_set_speed(motor_t *motor, uint32_t speed)
{
    if (speed < motor->speed_min || speed > motor->speed_max) {
        return -1;
    }
    motor->speed = speed;
    ioctl(motor->fd, MOTOR_SET_SPEED, &speed);
    return 0;
}

int motor_get_speed(motor_t *motor, uint32_t *speed)
{

    return 0;
}

int motor_set_angle(motor_t *motor, uint32_t angle)
{
    return 0;
}

int motor_get_angle(motor_t *motor, uint32_t *angle)
{
    return 0;
}


motor_t *motor_create(enum MOTOR_WORK_TYPE type, int id)
{
    motor_t *motor = calloc(1, sizeof(motor_t));
    if (motor == NULL) {
        return NULL;
    }
    char fname[12];
    switch (type) {
        case MOTOR_CURRENT_OPEN:
            snprintf(fname,12, "/dev/motor%d", id);
            motor->fd = open(fname, O_RDWR);
            if(motor->fd == 0) {
                log_error("Failed open the %s", fname);
            }
            break;
        case MOTOR_SPEED_RING:
            break;
        default:
            break;
    }
    motor->speed_max = 2000;
    motor->speed_min = 0;
    motor_set_speed(motor, 0);
    return motor;
}

void motor_destroy(motor_t *motor)
{
    if (motor == NULL)
        return;
    close(motor->fd);
    free(motor);
}