#include "sensor_platfrom.h"
#include "sensor.h"
#include <stdlib.h>

int sensor_register(sensor_t *sensor, enum SENSOR_TYPE type)
{
    int ret = 0;
    sensor->type = type;
    switch (type) {
        case SENSOR_TYPE_FAKE:
            ret = fake_sensor_register(sensor);
            break;
        case SENSOR_TYPE_MQ135:
            ret = mq135_sensor_register(sensor);
            break;
        case SENSOR_TYPE_DHT11:
            ret = dht11_sensor_register(sensor);
            break;
        case SENSOR_TYPE_SW180110P:
            ret = sw180110p_sensor_register(sensor);
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

sensor_t *sensor_crete_with_register(enum SENSOR_TYPE type)
{
    sensor_t *sensor;
    int ret = 0;
    sensor = malloc(sizeof(sensor_t));
    if (sensor == NULL) {
        return NULL;
    }
    ret = sensor_register(sensor, type);
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
    if (sensor->priv != NULL) {
        free(sensor->priv);
        sensor->priv = NULL;
    }
    free(sensor);
}