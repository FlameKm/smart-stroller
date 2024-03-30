#include "log.h"
#include "motor.h"
#include <unistd.h>

int main()
{
    motor_t *motor1 = motor_create(MOTOR_CURRENT_OPEN, 0);
    motor_t *motor2 = motor_create(MOTOR_CURRENT_OPEN, 1);

    if(motor1 == NULL || motor2 == NULL) {
        log_error("motor create failed");
        goto err_create;
    }

    for (int i = 0; i <= 10; i++) {
        motor_set_speed(motor1, 100 * i);
        motor_set_speed(motor2, 1000 - 100 * i);
        usleep(333 * 1000);
    }
    
    motor_set_speed(motor1, 0);
    motor_set_speed(motor2, 0);

err_create:
    motor_destroy(motor1);
    motor_destroy(motor2);
    return 0;
}