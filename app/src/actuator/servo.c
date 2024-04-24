#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "servo.h"
#include <stdio.h>

#define SERVO_SET_DUTY _IO('S', 0)//uint32_t
#define SERVO_GET_DUTY _IO('S', 1)
#define SERVO_START _IO('S', 2)
#define SERVO_STOP _IO('S', 3)

const char *servo_device = "/dev/servo";

typedef struct servo {
    int fd;
    int angle;
} servo_t;

static int servo_ioctl(servo_t *servo, int cmd, void *arg)
{
    int ret = 0;
    if (servo == NULL) {
        return -1;
    }
    ret = ioctl(servo->fd, cmd, arg);
    return ret;
}

int servo_start(servo_t *servo)
{
    return servo_ioctl(servo, SERVO_START, NULL);
}

int servo_stop(servo_t *servo)
{
    return servo_ioctl(servo, SERVO_STOP, NULL);
}


/**
 * @param angle 45.0 - 135.0 对应的是 1000 - 2000. 90度对应1500
 * @return Returns 0 on success, or an error code on failure.
 */
int servo_set_angle(servo_t *servo, float angle)
{
    int ret = 0;
    uint32_t duty;
    if (angle < 45.0 || angle > 135.0) {
        return -1;
    }
    duty = (angle - 45.0) / 90.0 * 1000 + 1000;
    ret = servo_ioctl(servo, SERVO_SET_DUTY, &duty);
    return ret;
}

int servo_get_angle(servo_t *servo, float *angle)
{
    int ret = 0;
    uint32_t duty;
    ret = servo_ioctl(servo, SERVO_GET_DUTY, &duty);
    if (ret < 0) {
        return -1;
    }
    *angle = (duty - 1000) / 1000.0 * 90.0 + 45.0;
    return ret;
}

servo_t *servo_create()
{
    servo_t *servo = calloc(1, sizeof(servo_t));
    if (servo == NULL) {
        return NULL;
    }
    servo->fd = open(servo_device, O_RDWR);
    if (servo->fd < 0) {
        free(servo);
        return NULL;
    }
    return servo;
}

void servo_destroy(servo_t *servo)
{
    if (servo == NULL)
        return;
    close(servo->fd);
    free(servo);
}