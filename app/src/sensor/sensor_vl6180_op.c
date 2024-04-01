#include <stdlib.h>
#include "sensor_op.h"
#include "hal_gpio.h"
#include "hal_iic.h"

/* data sheet: https://www.st.com/resource/en/datasheet/vl6180x.pdf */

typedef struct vl6180_data {
    void *null_data;
} vl6180_data_t;

static int vl6180_start_measure(vl6180_data_t *data)
{
    // todo 
    return 0;
}

static int vl6180_check_measure_done(vl6180_data_t *data)
{
    // todo
    return 0;
}

static int vl6180_init(sensor_t *sensor)
{
    sensor->priv = calloc(1, sizeof(vl6180_data_t));
    // todo
    return 0;
}

static int vl6180_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int vl6180_read(sensor_t *sensor, void *value, int channel)
{
    //todo
    return 0;
}

static int vl6180_config(sensor_t *sensor,  int cmd, unsigned long arg)
{
    //todo
    return 0;
}

static const sensor_op_t vl6180_op = {
    .init = vl6180_init,
    .deinit = vl6180_deinit,
    .read = vl6180_read,
    .config = vl6180_config,
};

int vl6180_sensor_register(sensor_t *sensor, void *iic, void *gpio_shut)
{
    sensor->op = &vl6180_op;
    //todo    
    return 0;
}