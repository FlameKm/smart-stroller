
#include "sensor_op.h"

int sensor_set_type(sensor_t *sensor, int type)
{
    sensor->type = type;
    return 0;
}

int sensor_read(sensor_t *sensor, void *value, int size)
{
    if (sensor->op->read)
        return sensor->op->read(sensor, value, size);
    else
        return -1;
}

int sensor_ioctl(sensor_t *sensor, void *value, int size)
{
    if (sensor->op->ioctl)
        return sensor->op->ioctl(sensor, value, size);
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