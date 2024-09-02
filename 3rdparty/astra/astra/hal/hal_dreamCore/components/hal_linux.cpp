//
// Created by Fir on 2024/2/12.
//
#include "astra/hal/hal_dreamCore/hal_dreamCore.h"
#include <csignal>
#include <sys/time.h>

void HALDreamCore::_delay(unsigned long _mill) {
    usleep(_mill * 1000);
}

int64_t HALDreamCore::_millis() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

unsigned long HALDreamCore::_getTick() {
    return _millis();
}

unsigned long HALDreamCore::_getRandomSeed() {
    return rd_();
}



