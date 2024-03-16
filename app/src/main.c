//
// Created by hyc on 24-2-4.
//
#include <stdio.h>
#include "stroller.h"

int main()
{
    stroller_t *strl;
    strl = strl_create();
    
    strl_start_loop(strl);

    strl_destroy(strl);
    printf("hello world\n");
}