//
// Created by wenyiyu on 2025/4/13.
//

#ifndef FAN_H
#define FAN_H
#include <stdbool.h>
#define PWM_FREQ 192
#define PWM_RANGE 1024
void init_fan();
void enable_fan(double rate);
#endif //FAN_H
