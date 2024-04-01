#include <stdlib.h>
#include "sensor_op.h"

typedef struct fake_data {
    void *null_data;
} fake_data_t;

static int fake_init(sensor_t *sensor)
{
    sensor->priv = calloc(1, sizeof(fake_data_t));
    return 0;
}

static int fake_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int fake_read(sensor_t *sensor, void *value, int channel)
{
    return 0;
}

static int fake_config(sensor_t *sensor,  int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t fake_op = {
    .init = fake_init,
    .deinit = fake_deinit,
    .read = fake_read,
    .config = fake_config,
};

int fake_sensor_register(sensor_t *sensor)
{
    sensor->op = &fake_op;
    return 0;
}