#include <unistd.h>
#include "log.h"
#include "hal_iic.h"
#include "sensor.h"
#include "sensor_platfrom.h"
#include <signal.h>
#include <stdbool.h>

bool is_stop = false;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        log_info("received SIGINT");
        is_stop = true;
    }
}

int main()
{
    int ret;
    sensor_t *sw18015;
    int shake;

    signal(SIGINT, sig_handler);
    sw18015 = sensor_create_with_register(SENSOR_TYPE_SW18015, NULL);
    if (sw18015 == NULL) {
        log_error("sensor_register failed");
        return -1;
    }

    while (!is_stop) {
        ret = sensor_read(sw18015, &shake, SENSOR_CHANNEL_DEFAULT);
        if (ret < 0) {
            log_error("sensor_read failed");
            break;
        }
        log_info("shake: %d", shake);
        sleep(1);
    }

    sensor_destroy(sw18015);
    return 0;
}