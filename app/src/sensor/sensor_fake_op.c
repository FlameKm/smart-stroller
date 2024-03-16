#include <stdlib.h>
#include <string.h>
#include "sensor_op.h"
#include "sensor_platfrom.h"

typedef struct fake_data {
    void *null_data;
} fake_data_t;

static int fake_init(sensor_t *sensor)
{
    return 0;
}

static int fake_deinit(sensor_t *sensor)
{
    return 0;
}

static int fake_read(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static int fake_ioctl(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static const sensor_op_t fake_op = {
    .init = fake_init,
    .deinit = fake_deinit,
    .read = fake_read,
    .ioctl = fake_ioctl,
};

int fake_sensor_register(sensor_t *sensor)
{
    if (sensor->type != SENSOR_TYPE_FAKE) {
        return -1;
    }
    sensor->priv = malloc(sizeof(fake_data_t));
    memset(sensor->priv, 0, sizeof(fake_data_t));
    sensor->op = &fake_op;
    return 0;
}