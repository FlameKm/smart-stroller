#include <stdlib.h>
#include "steering_gear.h"

typedef struct strg {
    void *temp;
} strg_t;

// todo


strg_t *strg_create()
{
    strg_t *strg = calloc(1, sizeof(strg_t));
    return strg;
}

void strg_destroy(strg_t *strg)
{
}