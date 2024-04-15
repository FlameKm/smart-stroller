#ifndef __SENSOR_H__
#define __SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sensor_platfrom.h"


#define SENSOR_CHANNEL0   0x0001
#define SENSOR_CHANNEL1   0x0002
#define SENSOR_CHANNEL2   0x0004
#define SENSOR_CHANNEL3   0x0008
#define SENSOR_CHANNEL4   0x0010
#define SENSOR_CHANNEL5   0x0020
#define SENSOR_CHANNEL6   0x0040
#define SENSOR_CHANNEL7   0x0080
#define SENSOR_CHANNEL_ALL   0x00FF
#define SENSOR_CHANNEL_NONE   0x0000
#define SENSOR_CHANNEL_DEFAULT   SENSOR_CHANNEL_ALL

#define SENSOR_MEASURE_MASK    0x8000
#define SENSOR_MEASURE_ENABLE  0x8000
#define SENSOR_MEASURE_DISABLE 0x0000

#define SENSOR_START_MEASURE   0x4000
#define SENSOR_CHEACK_MEASURE  0x0000

#define SENSOR_ENABLE 0x2000
#define SENSOR_DISABLE 0x1000

typedef struct sensor_op sensor_op_t;

typedef struct sensor {
    void *priv;
    void *arg;
    bool enabled;
    const sensor_op_t *op;
} sensor_t;

int sensor_read(sensor_t *sensor, void *value, int cmd);
int sensor_config(sensor_t *sensor,  int cmd, unsigned long arg);
int sensor_export(sensor_t *sensor);
int sensor_unexport(sensor_t *sensor);

sensor_t *sensor_create_with_register(enum SENSOR_TYPE type, void *custom);
int sensor_register(sensor_t *sensor, enum SENSOR_TYPE type, void *custom);
void sensor_destroy(sensor_t *sensor);
int sensor_is_enabled(sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif