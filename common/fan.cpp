//
// Created by wenyiyu on 2025/4/13.
//

#include "fan.h"

#include <wiringPi/wiringPi.h>
#include "log.h"

#define FAN_PIN 26

void init_fan()
{
    pinMode(FAN_PIN, PWM_OUTPUT);
    enable_fan(false);
}

void enable_fan(double rate)
{
#if !BUILD_SIMULATOR
    int value = (int)(PWM_RANGE * rate) % PWM_RANGE;
    LOG_INFO("enable_fan ==> %d", value);
    pwmWrite(FAN_PIN, value);
#endif
}
