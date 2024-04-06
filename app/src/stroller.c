#include "stroller.h"
#include "log.h"
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
        // todo, wating to async wake
        switch (chassis->mode) {
            case CHASSIS_MODE_AUTO:
                break;
            case CHASSIS_MODE_REMOTE:
                break;
            default:
                log_error("not find chassic mode %d", chassis->mode);
                break;
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
    strl = calloc(1, sizeof(stroller_t));
    if (strl == NULL) {
        log_error("Failed to calloc");
        goto err0;
    }

    strl->chassis.mr = motor_create(MOTOR_CURRENT_OPEN, 1);
    strl->chassis.ml = motor_create(MOTOR_CURRENT_OPEN, 2);
    strl->chassis.servo = servo_create();
    if (strl->chassis.mr == NULL || strl->chassis.ml == NULL || strl->chassis.servo == NULL) {
        log_error("Failed to create chassis");
        goto err1;
    }

    strl->sensor.aht10 = sensor_create_with_register(SENSOR_TYPE_AHT10, strl->iic);
    strl->sensor.mq135 = sensor_create_with_register(SENSOR_TYPE_MQ135, NULL);
    strl->sensor.sw18015 = sensor_create_with_register(SENSOR_TYPE_SW18015, NULL);
    if (strl->sensor.aht10 == NULL || strl->sensor.mq135 == NULL || strl->sensor.sw18015 == NULL) {
        log_error("Failed to create sensor");
        goto err2;
    }

    strl->comm.comm = comm_create_with_register(COMM_TYPE_UART);
    if(strl->comm.comm == NULL) {
        log_error("Failed to create strl");
        goto err3;
    }
    
    return strl;

err3:
    sensor_destroy(strl->sensor.aht10);
    sensor_destroy(strl->sensor.mq135);
    sensor_destroy(strl->sensor.sw18015);
err2:
    motor_destroy(strl->chassis.mr);
    motor_destroy(strl->chassis.ml);
    servo_destroy(strl->chassis.servo);
err1:
    free(strl);
err0:
    return NULL;
}

void strl_destroy(stroller_t *strl)
{
    comm_destroy(strl->comm.comm);

    sensor_destroy(strl->sensor.aht10);
    sensor_destroy(strl->sensor.mq135);
    sensor_destroy(strl->sensor.sw18015);

    motor_destroy(strl->chassis.mr);
    motor_destroy(strl->chassis.ml);
    servo_destroy(strl->chassis.servo);

    free(strl);
}