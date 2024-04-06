#ifndef __STEERING_GEAR_H__
#define __STEERING_GEAR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct servo servo_t;

int servo_start(servo_t *servo);
int servo_stop(servo_t *servo);
int servo_set_angle(servo_t *servo, float angle);
int servo_get_angle(servo_t *servo, float *angle);
servo_t *servo_create();
void servo_destroy(servo_t *servo);

#ifdef __cplusplus
}
#endif

#endif