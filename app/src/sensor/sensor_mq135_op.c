#include <stdlib.h>
#include "sensor_op.h"

typedef struct mq135_data {
    int data;
} mq135_data_t;

static int mq135_init(sensor_t *sensor)
{
    sensor->priv = calloc(1, sizeof(mq135_data_t));
    return 0;
}

static int mq135_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int mq135_read(sensor_t *sensor, void *value, int channel)
{
    return 0;
}

static int mq135_config(sensor_t *sensor,  int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t mq135_op = {
    .init = mq135_init,
    .deinit = mq135_deinit,
    .read = mq135_read,
    .config = mq135_config,
};

int mq135_sensor_register(sensor_t *sensor)
{
    sensor->op = &mq135_op;
    return 0;
}