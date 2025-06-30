//
// Created by wenyiyu on 2025/4/13.
//
#include "relay.h"

#include <wiringPi/wiringPi.h>
#define RELAY_PIN01 1
#define RELAY_PIN02 4

void init_relay()
{
    pinMode(RELAY_PIN01, OUTPUT);
    pinMode(RELAY_PIN02, OUTPUT);
    enable_relay01(false);
    enable_relay02(false);
}

void enable_relay01(bool enable)
{
#if !BUILD_SIMULATOR
    digitalWrite(RELAY_PIN01, enable ? HIGH : LOW);
#endif
}

void enable_relay02(bool enable)
{
#if !BUILD_SIMULATOR
    digitalWrite(RELAY_PIN02, enable ? HIGH : LOW);
#endif
}
