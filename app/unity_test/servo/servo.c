#include "servo.h"
#include <signal.h>
#include <unistd.h>
#include "log.h"

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
    int ret = 0;
    signal(SIGINT, sig_handler);
    servo_t *servo = servo_create();
    char input;
    float angle = 90.0;
    servo_set_angle(servo, angle);
    servo_start(servo);
    while (!is_stop) {
        input = getchar();
        if (input == 'q') {
            break;
        }
        else if (input == '+') {
            angle += 5.0;
        }
        else if (input == '-') {
            angle -= 5.0;
        }
        else {
            log_warn("not find %c, please press 'q' to break", input);
            continue;
        }
        ret = servo_set_angle(servo, angle);
        log_debug("set angle %.1f, ret %d", angle, ret);
    }

err:
    servo_stop(servo);
    servo_destroy(servo);
    return 0;
}


/* int servo_test(servo_t *servo)
{
    int ret = 0;
    servo_start(servo);

    char input;
    int duty = 1200;
    while (1) {
        input = getchar();
        if (input == 'q') {
            break;
        }
        else if (input == '+') {
            duty += 100;
        }
        else if (input == '-') {
            duty -= 100;
        }else {
            continue;
        }

        ret = ioctl(servo->fd, SERVO_SET_DUTY, &duty);
        log_debug("set duty %d, ret %d", duty, ret);
    }
    servo_stop(servo);
} */