#include "log.h"
#include "chassis.h"
#include <unistd.h>

int main()
{
    int ret = 0;
    chassis_t chassis;
    ret = chassis_register(&chassis);
    if (ret < 0) {
        log_error("chassis register failed");
        goto err;
    }


    for (int i = 0; i <= 10; i++) {
        float s1, s2;
        s1 = 100 * i;
        s2 = 1000 - 100 * i;
        ret = set_chassis_speed(&chassis, s1);
        if (ret < 0) {
            log_error("motor set speed failed");
            goto err;
        }
        log_info("motor1 set to %.1f, motor2 set to %.1f", s1, s2);
        usleep(333 * 1000);
    }

    for (int i = 0; i <= 10; i++) {
        float turn;
        turn = 5.0 * (i - 5);
        ret = set_chassis_turn(&chassis, turn);
        if (ret < 0) {
            log_error("motor set trun failed");
            goto err;
        }
        log_info("turn %.1f", turn);
        usleep(333 * 1000);
    }

    set_chassis_speed(&chassis, 0);

err:
    chassis_destroy(&chassis);
    return 0;
}