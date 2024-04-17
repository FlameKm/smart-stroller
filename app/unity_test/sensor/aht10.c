#include <unistd.h>
#include "log.h"
#include "hal_iic.h"
#include "sensor.h"
#include "sensor_platfrom.h"

int main()
{
    int ret;
    iic_dev_t *iic;
    sensor_t *aht10;
    float temperature;
    float humidity;

    iic = iic_create(3);
    if(iic == NULL)
    {
        log_error("iic_create failed");
        return -1;
    }
    aht10 = sensor_create_with_register( SENSOR_TYPE_AHT10, iic);
    if (ret < 0) {
        log_error("sensor_register failed");
        return -1;
    }
    ret = sensor_read(aht10, &temperature, SENSOR_CHANNEL0 | SENSOR_MEASURE_ENABLE);
    usleep(100 * 1000); // ! Need interval
    ret |= sensor_read(aht10, &humidity, SENSOR_CHANNEL1 | SENSOR_MEASURE_DISABLE);
    if (ret < 0) {
        log_error("sensor_read failed");
        return -1;
    }

    log_info("temperature: %.2f humidity: %.2f", temperature, humidity);
    sensor_destroy(aht10);
    iic_destroy(iic);
    return 0;
}