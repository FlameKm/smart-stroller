#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "sensor.h"
#include "sensor_op.h"
#include "hal_gpio.h"
#include "hal_iic.h"

/* data sheet: https://www.st.com/resource/en/datasheet/vl6180x.pdf */
static const int iic_addr[3] = {0x29, 0x59, 0x5a};
static int device_cnt;

typedef struct vl6180_data {
    gpio_dev_t *shut;
    iic_dev_t *iic;
    int addr;
    int port;
    int dest_addr;
} vl6180_data_t;

static int vl6180_change_addr(vl6180_data_t *data, uint8_t to_addr)
{
    int ret;
    log_info("vl6180 address set to 0x%x", to_addr);
    ret = iic_reg16_write(data->iic, data->addr, 0x0212, &to_addr, 1);
    if (ret < 0) {
        return -1;
    }
    uint8_t recv;
    ret = iic_reg16_read(data->iic, to_addr, 0x0212, &recv, 1);
    if (recv != to_addr) {
        return -2;
    }
    data->addr = to_addr;
    return 0;
}

static int vl6180_enable(vl6180_data_t *data)
{
    int ret = 0;
    gpio_set_value(data->shut, 1);
    ret = vl6180_change_addr(data, data->dest_addr);
    return ret;
}

static int vl6180_disable(vl6180_data_t *data)
{
    gpio_set_value(data->shut, 0);
    return 0;
}

static int vl6180_start_measure(vl6180_data_t *data)
{
    int ret = 0;
    uint8_t value = 0x01;
    ret = iic_reg16_write(data->iic, data->addr, 0x0018, &value, 1);
    return ret;
}

static int vl6180_read_measure(vl6180_data_t *data, uint16_t *value)
{
    int ret = 0;
    uint8_t read = 0;
    ret = iic_reg16_read(data->iic, data->addr, 0x0062, &read, 1);
    if (ret < 0) {
        return -1;
    }
    *value = read;
    return ret;
}

static int vl6180_check_measure_done(vl6180_data_t *data)
{
    int ret = 0;
    uint8_t status, range_status;
    ret = iic_reg16_read(data->iic, data->addr, 0x004f, &status, 1);
    if (ret < 0) {
        return -1;
    }
    range_status = status & 0x07;
    if (range_status != 0x04) {
        return -1;
    }
    return 0;
}

static int vl6180_init(sensor_t *sensor)
{
    struct vl6180_data *data = sensor->priv;
    data->shut = gpio_create(data->port, GPIO_DIRECTION_OUT);
    if (data->shut == NULL) {
        return -1;
    }
    data->addr = iic_addr[0];
    data->dest_addr = iic_addr[++device_cnt];
    return 0;
}

static int vl6180_deinit(sensor_t *sensor)
{
    struct vl6180_data *data = sensor->priv;
    vl6180_disable(data);
    gpio_destroy(data->shut);
    free(sensor->priv);
    return 0;
}

static int vl6180_read(sensor_t *sensor, void *value, int channel)
{
    struct vl6180_data *data = sensor->priv;
    uint16_t read;

    switch (channel) {
        case SENSOR_MEASURE_ENABLE:
            vl6180_start_measure(data);
            while (vl6180_check_measure_done(data) != 0) {
                usleep(100);
            }
            break;
        case SENSOR_MEASURE_DISABLE:
            break;
        default:
            break;
    }
    int ret = vl6180_read_measure(data, &read);
    if (ret < 0) {
        log_error("vl6180 read failed");
        return -1;
    }

    *(int *)value = read;
    return 0;
}

static int vl6180_config(sensor_t *sensor, int cmd, unsigned long arg)
{
    int ret = 0;
    struct vl6180_data *data = sensor->priv;
    switch (cmd) {
        case SENSOR_START_MEASURE:
            vl6180_start_measure(data);
            break;
        case SENSOR_CHEACK_MEASURE:
            ret = vl6180_check_measure_done(data);
            *(int *)arg = ret;
            break;
        case SENSOR_ENABLE:
            vl6180_enable(data);
            break;
        case SENSOR_DISABLE:
            vl6180_disable(data);
            break;
    }
    return 0;
}

static const sensor_op_t vl6180_op = {
    .init = vl6180_init,
    .deinit = vl6180_deinit,
    .read = vl6180_read,
    .config = vl6180_config,
};

int vl6180_sensor_register(sensor_t *sensor, void *iic, int gpio_port)
{
    if (sensor == NULL || iic == NULL) {
        return -1;
    }
    vl6180_data_t *data = calloc(1, sizeof(vl6180_data_t));
    data->iic = iic;
    data->port = gpio_port;

    sensor->op = &vl6180_op;
    sensor->priv = data;

    return 0;
}