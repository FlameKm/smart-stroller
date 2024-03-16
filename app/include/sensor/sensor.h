#ifndef __SENSOR_H__
#define __SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor_platfrom.h"

typedef struct sensor_op sensor_op_t;

typedef struct sensor {
    int type;
    void *priv;
    const sensor_op_t *op;
} sensor_t;

int sensor_read(sensor_t *sensor, void *value, int size);
int sensor_ioctl(sensor_t *sensor, void *value, int size);
int sensor_export(sensor_t *sensor);
int sensor_unexport(sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif