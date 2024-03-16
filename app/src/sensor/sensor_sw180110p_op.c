#include "sensor_op.h"
#include "sensor_platfrom.h"
#include <stdlib.h>
#include <string.h>

typedef struct sw180110p_data {
    int data;
} sw180110p_data_t;

static int sw180110p_init(sensor_t *sensor)
{
    return 0;
}

static int sw180110p_deinit(sensor_t *sensor)
{
    return 0;
}

static int sw180110p_read(sensor_t *sensor, void *value, int size)
{
    return 0;
}

static int sw180110p_ioctl(sensor_t *sensor, void *value, int size)
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
    if (sensor->type != SENSOR_TYPE_SW180110P) {
        return -1;
    }
    sensor->priv = malloc(sizeof(sw180110p_data_t));
    memset(sensor->priv, 0, sizeof(sw180110p_data_t));
    sensor->op = &sw180110p_op;
    return 0;
}