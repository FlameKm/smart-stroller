#ifndef __SENSOR_OP_H__
#define __SENSOR_OP_H__

#include "sensor.h"

typedef struct sensor_op {
    int (*init)(sensor_t *sensor);
    int (*deinit)(sensor_t *sensor);
    int (*read)(sensor_t *sensor, void *value, int size);
    int (*ioctl)(sensor_t *sensor, void *value, int size);
} sensor_op_t;

#endif