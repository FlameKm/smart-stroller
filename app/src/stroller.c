#include "stroller.h"
#include "stdlib.h"
#include <unistd.h>

static void sensor_loop(void *ptr)
{
    stroller_t *strl = ptr;
    stlr_sensor_t *sensor = &strl->sensor;
    while (1) {

        sleep(1);
    }
}

static void follow_loop(void *ptr)
{
    stroller_t *strl = ptr;
    stlr_chassis_t *chassis = &strl->chassis;
    while (1) {
        // todo, wating to async wake,
        switch (chassis->mode) {
        }
    }
}

static void comm_loop(void *ptr)
{
    stroller_t *strl = ptr;
    stlr_comm_t *comm = &strl->comm;
    while (1) {
        // todo, wating to async wake, then to work events.
    }
}

int strl_start_loop(stroller_t *strl)
{

    return 0;
}

stroller_t *strl_create()
{
    stroller_t *strl;
    strl = malloc(sizeof(stroller_t));
    if (strl == NULL) {
        goto err0;
    }

    strl->chassis.mr = motor_create(MOTOR_CURRENT_OPEN);
    strl->chassis.ml = motor_create(MOTOR_CURRENT_OPEN);
    strl->chassis.strg = strg_create();
    if (strl->chassis.mr == NULL || strl->chassis.ml == NULL || strl->chassis.strg == NULL) {
        goto err1;
    }

    strl->sensor.dht11 = sensor_crete_with_register(SENSOR_TYPE_DHT11);
    strl->sensor.mq135 = sensor_crete_with_register(SENSOR_TYPE_MQ135);
    strl->sensor.sw180110p = sensor_crete_with_register(SENSOR_TYPE_SW180110P);
    if (strl->sensor.dht11 == NULL || strl->sensor.mq135 == NULL || strl->sensor.sw180110p == NULL) {
        goto err1;
    }

    return strl;

err2:
    sensor_destroy(strl->sensor.dht11);
    sensor_destroy(strl->sensor.mq135);
    sensor_destroy(strl->sensor.sw180110p);
err1:
    motor_destroy(strl->chassis.mr);
    motor_destroy(strl->chassis.ml);
    strg_destroy(strl->chassis.strg);
err0:
    return NULL;
}

void strl_destroy(stroller_t *strl)
{
    sensor_destroy(strl->sensor.dht11);
    sensor_destroy(strl->sensor.mq135);
    sensor_destroy(strl->sensor.sw180110p);

    motor_destroy(strl->chassis.mr);
    motor_destroy(strl->chassis.ml);
    strg_destroy(strl->chassis.strg);

    free(strl);
}