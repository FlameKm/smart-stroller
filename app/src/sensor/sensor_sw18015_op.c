#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "sensor.h"
#include "sensor_op.h"

#define SW18015_SHAKE_COUNT _IOR('k', 0, int)
#define SW18015_CLEAN _IOR('k', 1, int)
typedef struct sw18015_data {
    int fd;
    int value;
} sw18015_data_t;

const char *sw18015_name = "/dev/sw18015";

static int sw18015_init(sensor_t *sensor)
{
    sw18015_data_t *data = calloc(1, sizeof(sw18015_data_t));
    data->fd = open(sw18015_name, O_RDWR);
    if (data->fd < 0) {
        free(data);
        return -1;
    }
    sensor->priv = data;
    return 0;
}

static int sw18015_deinit(sensor_t *sensor)
{
    struct sw18015_data *data = sensor->priv;
    close(data->fd);
    free(sensor->priv);
    return 0;
}

static int sw18015_read(sensor_t *sensor, void *value, int channel)
{
    sw18015_data_t *data = sensor->priv;
    int ret = 0;

    if (channel == SENSOR_CHANNEL0 | channel == SENSOR_CHANNEL_DEFAULT) {
        ret = ioctl(data->fd, SW18015_SHAKE_COUNT, &data->value);
        if (ret < 0) {
            return -1;
        }
        ret = ioctl(data->fd, SW18015_CLEAN, NULL);
        *(int *)value = data->value;
    }
    return ret;
}

static int sw18015_config(sensor_t *sensor, int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t sw18015_op = {
    .init = sw18015_init,
    .deinit = sw18015_deinit,
    .read = sw18015_read,
    .config = sw18015_config,
};

int sw18015_sensor_register(sensor_t *sensor)
{
    sensor->op = &sw18015_op;
    return 0;
}