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
    SENSOR_TYPE_SW180110P,
} SENSOR_TYPE;

typedef struct sensor sensor_t;

int fake_sensor_register(sensor_t *sensor);
int aht10_sensor_register(sensor_t *sensor, void *iic);
int mq135_sensor_register(sensor_t *sensor);
int sw180110p_sensor_register(sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif