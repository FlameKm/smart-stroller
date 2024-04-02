#include <stdint.h>
#include <unistd.h>
#include "log.h"
#include "hal_iic.h"
#include "sensor.h"
#include "sensor_platfrom.h"

int main()
{
    int ret;
    iic_dev_t *iic;
    sensor_t *vl61801, *vl61802;
    uint8_t distance1, distance2;
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
    sensor_config(vl61801, SENSOR_ENABLE, NULL);
    sensor_config(vl61801, SENSOR_START_MEASURE, NULL);
    log_info("start measure");
    usleep(100 * 1000);

    int flag;
    sensor_config(vl61801, SENSOR_CHEACK_MEASURE, (uint64_t)(&flag));
    log_info("sensor cheack measure flag %d", flag);

    sensor_read(vl61801, &distance1, SENSOR_MEASURE_DISABLE);

    log_info("distance1: %d distance2: %d", distance1, distance2);
    sensor_destroy(vl61801);
    iic_destroy(iic);
    return 0;
}