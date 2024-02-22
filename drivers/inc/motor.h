#ifndef _MOTOR_H
#define _MOTOR_H

#include <stdint.h>
struct motor_struct {
    uint16_t speed;
    uint8_t direction;
    uint16_t max;
    uint16_t min;
};

#endif