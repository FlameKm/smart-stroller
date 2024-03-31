#include "sensor_op.h"
#include <stdlib.h>

typedef struct sw180110p_data {
    int data;
} sw180110p_data_t;

static int sw180110p_init(sensor_t *sensor)
{
    sensor->priv = calloc(1, sizeof(sw180110p_data_t));
    return 0;
}

static int sw180110p_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int sw180110p_read(sensor_t *sensor, void *value, int channel)
{
    return 0;
}

static int sw180110p_ioctl(sensor_t *sensor,  int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t sw180110p_op = {
    .init = sw180110p_init,
    .deinit = sw180110p_deinit,
    .read = sw180110p_read,
    .ioctl = sw180110p_ioctl,
};

int sw180110p_sensor_register(sensor_t *sensor)
{
    sensor->op = &sw180110p_op;
    return 0;
}