#ifndef __SENSOR_PLATFROM_H__
#define __SENSOR_PLATFROM_H__

#include "sensor.h"

typedef struct sensor_op sensor_op_t;

int xxx_sensor_register(sensor_t *sensor);
int fake_sensor_register(sensor_t *sensor);


#endif