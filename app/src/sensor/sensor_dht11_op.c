#include <stdlib.h>
#include <string.h>
#include "sensor_op.h"
#include "sensor_platfrom.h"

typedef struct dht11_data {
    int data;
} dht11_data_t;

static int dht11_init(sensor_t *sensor)
{
    return 0;
}

static int dht11_deinit(sensor_t *sensor)
{
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
    if (sensor->type != SENSOR_TYPE_DHT11) {
        return -1;
    }
    sensor->priv = malloc(sizeof(dht11_data_t));
    memset(sensor->priv, 0, sizeof(dht11_data_t));
    sensor->op = &dht11_op;
    return 0;
}