#ifndef __STROLLER_H__
#define __STROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "motor.h"
#include "sensor.h"
#include "steering_gear.h"

typedef struct stlr_sensor {
    sensor_t *dht11;
    sensor_t *mq135;
    sensor_t *sw180110p;
} stlr_sensor_t;

typedef struct stlr_chassis {
    int mode;
    motor_t *ml;
    motor_t *mr;
    strg_t *strg;
} stlr_chassis_t;

typedef struct stlr_comm {
    void *temporaly;
} stlr_comm_t;

typedef struct stroller {
    stlr_chassis_t chassis;
    stlr_sensor_t sensor;
    stlr_comm_t comm;
} stroller_t;


stroller_t *strl_create();
void strl_destroy(stroller_t *strl);
int strl_start_loop(stroller_t *strl);

#ifdef __cplusplus
}
#endif

#endif