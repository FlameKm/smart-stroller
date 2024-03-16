#include <stdlib.h>
#include <string.h>
#include "sensor_op.h"
#include "sensor_platfrom.h"

typedef struct mq135_data {
    int data;
} mq135_data_t;

static int mq135_init(sensor_t *sensor)
{
    return 0;
}

static int mq135_deinit(sensor_t *sensor)
{
    return 0;
}

static int mq135_read(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static int mq135_ioctl(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static const sensor_op_t mq135_op = {
    .init = mq135_init,
    .deinit = mq135_deinit,
    .read = mq135_read,
    .ioctl = mq135_ioctl,
};

int mq135_sensor_register(sensor_t *sensor)
{
    if (sensor->type != SENSOR_TYPE_MQ135) {
        return -1;
    }
    sensor->priv = malloc(sizeof(mq135_data_t));
    memset(sensor->priv, 0, sizeof(mq135_data_t));
    sensor->op = &mq135_op;
    return 0;
}