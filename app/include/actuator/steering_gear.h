#ifndef __STEERING_GEAR_H__
#define __STEERING_GEAR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct strg strg_t;

int strg_set_direction(strg_t *strg, float angle);
int strg_get_direction(strg_t *strg, float angle);
strg_t *strg_create();
void strg_destroy(strg_t *strg);

#ifdef __cplusplus
}
#endif

#endif