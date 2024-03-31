#include "log.h"
#include "hal_iic.h"
#include "sensor.h"
#include <unistd.h>

int main()
{
    int ret;
    iic_dev_t *iic;
    sensor_t *aht10;
    float temperature;
    float humidity;

    iic = iic_create(2);
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
    ret = sensor_read(aht10, &temperature, 0);
    usleep(1000);
    ret |= sensor_read(aht10, &humidity, 1);
    if (ret < 0) {
        log_error("sensor_read failed");
        return -1;
    }

    log_info("temperature: %.2f humidity: %.2f", temperature, humidity);
    sensor_destroy(aht10);
    iic_destroy(iic);
    return 0;
}