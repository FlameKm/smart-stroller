#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include "log.h"
#include "hal_iic.h"
#include "sensor.h"
#include "sensor_platfrom.h"

int contine = true;
void stop(int signo)
{
    log_error("oops! stop!!!");
    contine = false;
}

int main()
{
    int ret;
    iic_dev_t *iic;
    sensor_t *vl61801, *vl61802;
    uint8_t distance1, distance2;
    signal(SIGINT, stop);

    iic = iic_create(2);
    if (iic == NULL) {
        log_error("iic_create failed");
        return -1;
    }

    vl61801 = sensor_create_with_register(SENSOR_TYPE_VL6180_1, iic);
    vl61802 = sensor_create_with_register(SENSOR_TYPE_VL6180_2, iic);
    if (vl61801 == NULL || vl61802 == NULL) {
        log_error("sensor_create failed");
        return -1;
    }
    ret = sensor_config(vl61801, SENSOR_ENABLE, 0);
    ret = sensor_config(vl61802, SENSOR_ENABLE, 0);


    while (contine) {
        ret = sensor_config(vl61801, SENSOR_START_MEASURE, 0);
        ret |= sensor_config(vl61802, SENSOR_START_MEASURE, 0);
        if (ret < 0) {
            log_error("sensor_config SENSOR_START_MEASURE failed");
            break;
        }
        usleep(100);

        int flag1, flag2;
        ret = sensor_config(vl61801, SENSOR_CHEACK_MEASURE, (uint64_t)(&flag1));
        ret |= sensor_config(vl61802, SENSOR_CHEACK_MEASURE, (uint64_t)(&flag2));
        if (ret < 0) {
            log_error("sensor_config SENSOR_CHEACK_MEASURE failed");
            break;
        }

        ret = sensor_read(vl61801, &distance1, SENSOR_MEASURE_DISABLE);
        ret |= sensor_read(vl61802, &distance2, SENSOR_MEASURE_DISABLE);
        if (ret < 0) {
            log_error("sensor_read failed");
            break;
        }

        log_info("flag %d %d, distance1: %d distance2: %d", flag1, flag2, distance1, distance2);
        usleep(333 * 1000);
    }

    sensor_destroy(vl61801);
    sensor_destroy(vl61802);
    iic_destroy(iic);
    return 0;
}