#include "sensor_platfrom.h"
#include "sensor_op.h"
#include <stdlib.h>
#include "hal_iic.h"

int sensor_register(sensor_t *sensor, enum SENSOR_TYPE type, void *custom)
{
    int ret = 0;
    sensor->enabled = false;
    switch (type) {
        case SENSOR_TYPE_FAKE:
            ret = fake_sensor_register(sensor);
            break;
        case SENSOR_TYPE_MQ135:
            ret = mq135_sensor_register(sensor);
            break;
        case SENSOR_TYPE_AHT10: {
            iic_dev_t *iic = (iic_dev_t *)custom;
            ret = aht10_sensor_register(sensor, custom);
            break;
        }
        case SENSOR_TYPE_SW18015:
            ret = sw18015_sensor_register(sensor);
            break;
        case SENSOR_TYPE_HLK2411S:
            ret = hlk2411s_sensor_register(sensor);
            break;
        case SENSOR_TYPE_VL6180_1: {
            iic_dev_t *iic = (iic_dev_t *)custom;
            ret = vl6180_sensor_register(sensor, iic, 74);
            break;
        }
        case SENSOR_TYPE_VL6180_2: {
            iic_dev_t *iic = (iic_dev_t *)custom;
            ret = vl6180_sensor_register(sensor, iic, 233);
            break;
        }
        default:
            ret = -1;
            break;
    }
    if (!ret) {
        ret = sensor->op->init(sensor);
        if (!ret && type != SENSOR_TYPE_FAKE) {
            sensor->enabled = (!ret ? true : false);
        }
    }
    return ret;
}

sensor_t *sensor_create_with_register(enum SENSOR_TYPE type, void *custom)
{
    sensor_t *sensor;
    int ret = 0;
    sensor = calloc(1, sizeof(sensor_t));
    if (sensor == NULL) {
        return NULL;
    }
    ret = sensor_register(sensor, type, custom);
    if (ret < 0) {
        free(sensor);
        return NULL;
    }
    return sensor;
}

void sensor_destroy(sensor_t *sensor)
{
    if (sensor == NULL) {
        return;
    }
    sensor->op->deinit(sensor);
    free(sensor);
}

int sensor_is_enabled(sensor_t *sensor)
{
    return sensor->enabled;
}