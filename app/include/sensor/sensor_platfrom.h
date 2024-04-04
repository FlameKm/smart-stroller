#ifndef __SENSOR_PLATFROM_H__
#define __SENSOR_PLATFROM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sensor_op sensor_op_t;

typedef enum SENSOR_TYPE
{
    SENSOR_TYPE_FAKE,
    SENSOR_TYPE_AHT10,
    SENSOR_TYPE_MQ135,
    SENSOR_TYPE_SW18015,
    SENSOR_TYPE_VL6180_1,
    SENSOR_TYPE_VL6180_2,
} SENSOR_TYPE;

typedef struct sensor sensor_t;

int fake_sensor_register(sensor_t *sensor);
int aht10_sensor_register(sensor_t *sensor, void *iic);
int mq135_sensor_register(sensor_t *sensor);
int sw18015_sensor_register(sensor_t *sensor);
int vl6180_sensor_register(sensor_t *sensor, void *iic, int gpio_port);

#ifdef __cplusplus
}
#endif

#endif