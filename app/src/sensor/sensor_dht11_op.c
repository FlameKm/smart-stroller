#include <stdlib.h>
#include "sensor_op.h"
typedef struct dht11_data {
    int data;
} dht11_data_t;

static int dht11_init(sensor_t *sensor)
{
    sensor->priv = calloc(1, sizeof(dht11_data_t));
    return 0;
}

static int dht11_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int dht11_read(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static int dht11_ioctl(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static const sensor_op_t dht11_op = {
    .init = dht11_init,
    .deinit = dht11_deinit,
    .read = dht11_read,
    .ioctl = dht11_ioctl,
};

int dht11_sensor_register(sensor_t *sensor)
{
    sensor->op = &dht11_op;
    return 0;
}