//
// Created by wenyiyu on 2024/9/2.
//

#ifndef RASPBERRY_PI_OLED_PANEL_CLOCKER_H
#define RASPBERRY_PI_OLED_PANEL_CLOCKER_H

#include <cstdint>

namespace astra {
    class Clocker {
    public:
       explicit Clocker();

        void start();

        void pause();

        void next();

        int64_t currentDuration();

        int64_t lastDuration();

        void reset();

        void waitUntil(int64_t except);
    private:
        int64_t last_time_{0};
        int64_t duration_{0};
        int64_t last_duration_{0};
    };
}
#endif //RASPBERRY_PI_OLED_PANEL_CLOCKER_H
