//
// Created by wenyiyu on 2024/9/1.
//

#ifndef RASPBERRY_PI_OLED_PANEL_SH1106_HAL_H
#define RASPBERRY_PI_OLED_PANEL_SH1106_HAL_H

#include <astra/hal/hal_dreamCore/hal_dreamCore.h>
#include <periphery/gpio.h>

class Sh1106Hal : public HALDreamCore {
public:
    Sh1106Hal() = default;
    bool _getKey(key::KEY_INDEX _keyIndex) override;
    void _screenOn() override;
    void _screenOff() override;
    void init() override;
private:
    gpio_t * keys_[key::KEY_NUM];
};


#endif //RASPBERRY_PI_OLED_PANEL_SH1106_HAL_H
