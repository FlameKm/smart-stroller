#include "sensor_op.h"
#include <stdio.h>

static const sensor_op_t fake_op;

int fake_sensor_register(sensor_t *sensor)
{
    sensor->priv = NULL;
    sensor->op = &fake_op;
    return 0;
}