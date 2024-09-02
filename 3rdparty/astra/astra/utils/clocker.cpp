//
// Created by wenyiyu on 2024/9/2.
//

#include "astra/utils/clocker.h"
#include "astra/hal/hal.h"

namespace astra {
    Clocker::Clocker() : last_time_(HAL::millis()) {}

    void Clocker::start() {
        last_time_ = HAL::millis();
    }

    void Clocker::pause() {
        duration_ += HAL::millis() - last_time_;
        last_time_ = -1;
    }

    void Clocker::next() {
        auto current = HAL::millis();
        last_duration_ = duration_ + current - last_time_;
        last_time_ = current;
        duration_ = 0;
    }

    int64_t Clocker::currentDuration() {
        if (last_time_ < 0) {
            return duration_;
        }
        auto current = HAL::millis();
        return duration_ + current - last_time_;
    }

    int64_t Clocker::lastDuration() {
        return last_duration_;
    }

    void Clocker::reset() {
        last_duration_ = 0;
        duration_ = 0;
        last_time_ = HAL::millis();
    }
}
