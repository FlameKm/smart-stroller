#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "sensor_op.h"
#include "hal_iic.h"


#define AHT10_IIC_ADRESS 0x38

#define AHT10_CMD_INIT 0b11100001
#define AHT10_CMD_MEAS 0b10101100
#define AHT10_CMD_RST 0b10111010

typedef struct aht10_data {
    iic_dev_t *iic;
    int addr;
    int data;
    float temperature;
    float humidity;
} aht10_data_t;

static int aht10_init(sensor_t *sensor)
{
    aht10_data_t *data;
    iic_dev_t *iic = sensor->arg;

    data = (aht10_data_t *)calloc(1, sizeof(aht10_data_t));
    if (data == NULL) {
        return -1;
    }

    data->iic = iic;
    data->addr = AHT10_IIC_ADRESS;
    sensor->priv = data;
    return 0;
}

static int aht10_deinit(sensor_t *sensor)
{
    free(sensor->priv);
    return 0;
}

static int aht10_read(sensor_t *sensor, void *value, int channel)
{
    aht10_data_t *data = sensor->priv;
    const uint8_t cmd_meas[] = {AHT10_CMD_MEAS, 0x33, 0x00};
    uint8_t raw_data[6] = {0};
    uint32_t humi, temp;

    iic_write(data->iic, data->addr, cmd_meas, sizeof(cmd_meas));
    iic_read(data->iic, data->addr, raw_data, sizeof(raw_data));

    if((raw_data[0] & 0x80) == 0) {
        log_error("aht10 read error, status: %02x", raw_data[0]);
        return -1;
    }

    humi = (raw_data[1] << 12) | (raw_data[2] << 4) | (raw_data[3] >> 4);
    temp = ((raw_data[3] & 0X0F) << 16) | (raw_data[4] << 8) | (raw_data[5]);

    data->humidity = humi * 100.0 / 1024 / 1024 + 0.5;
    data->temperature = (temp * 2000.0 / 1024 / 1024 + 0.5) / 10.0 - 50;

    switch (channel) {
        case 0:
            *(float *)value = data->temperature;
            break;
        case 1:
            *(float *)value = data->humidity;
            break;
        default:
            return -1;
    }
    return 0;
}

static int aht10_ioctl(sensor_t *sensor, int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t aht10_op = {
    .init = aht10_init,
    .deinit = aht10_deinit,
    .read = aht10_read,
    .ioctl = aht10_ioctl,
};

int aht10_sensor_register(sensor_t *sensor, void *iic)
{
    sensor->op = &aht10_op;
    sensor->arg = iic;
    return 0;
}