#include "log.h"
#include "motor.h"
#include <unistd.h>

int main()
{
    int ret = 0;
    motor_t *motor1 = motor_create(MOTOR_CURRENT_OPEN, 0);
    motor_t *motor2 = motor_create(MOTOR_CURRENT_OPEN, 1);

    if(motor1 == NULL || motor2 == NULL) {
        log_error("motor create failed");
        goto err;
    }

    for (int i = 0; i <= 10; i++) {
        int s1, s2;
        s1 = 100 * i;
        s2 = 1000 - 100 * i;
        ret = motor_set_speed(motor1, s1);
        ret = motor_set_speed(motor2, s2);
        if(ret < 0) {
            log_error("motor set speed failed");
            goto err;
        }
        log_info("motor1 set to %d, motor2 set to %d", s1, s2);
        usleep(333 * 1000);
    }
    
    motor_set_speed(motor1, 0);
    motor_set_speed(motor2, 0);

err:
    motor_destroy(motor1);
    motor_destroy(motor2);
    return 0;
}