
#include "sensor_op.h"

int sensor_read(sensor_t *sensor, void *value, int channel)
{
    if (sensor->op->read)
        return sensor->op->read(sensor, value, channel);
    else
        return -1;
}

int sensor_config(sensor_t *sensor, int cmd, unsigned long arg)
{
    if (sensor->op->config)
        return sensor->op->config(sensor, cmd, arg);
    else
        return -1;
}

int sensor_export(sensor_t *sensor)
{
    if (sensor->op->init)
        return sensor->op->init(sensor);
    else
        return -1;
}

int sensor_unexport(sensor_t *sensor)
{
    if (sensor->op->deinit)
        return sensor->op->deinit(sensor);
    else
        return -1;
}