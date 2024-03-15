#include <stdlib.h>
#include <string.h>
#include "sensor_op.h"
typedef struct xxx_data {
    int data;
} xxx_data_t;

static int xxx_init(sensor_t *sensor)
{
    return 0;
}

static int xxx_deinit(sensor_t *sensor)
{
    return 0;
}

static int xxx_read(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static int xxx_ioctl(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static const sensor_op_t xxx_op = {
    .init = xxx_init,
    .deinit = xxx_deinit,
    .read = xxx_read,
    .ioctl = xxx_ioctl,
};

int xxx_sensor_register(sensor_t *sensor)
{
    sensor->priv = malloc(sizeof(xxx_data_t));
    memset(sensor->priv, 0, sizeof(xxx_data_t));
    sensor->op = &xxx_op;
    return 0;
}