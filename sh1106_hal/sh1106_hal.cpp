//
// Created by wenyiyu on 2024/9/1.
//

#include "sh1106_hal.h"
#include <u8g2port.h>
#include <wiringPi/wiringPi.h>

// Set I2C bus and address
#define I2C_BUS 1
#define I2C_ADDRESS (0x3c * 2)
#define GPIO_KEY_PREV 26
#define GPIO_KEY_NEXT 25
#define GPIO_KEY_CONFIRM 16
#define GPIO_KEY_CANCEL 24

static std::vector HAL_GPIO_PINS{GPIO_KEY_PREV, GPIO_KEY_NEXT, GPIO_KEY_CONFIRM, GPIO_KEY_CANCEL};

void Sh1106Hal::init() {
    HALDreamCore::init();
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&canvasBuffer,
                                          U8G2_R0,
                                          u8x8_byte_arm_linux_hw_i2c,
                                          u8x8_arm_linux_gpio_and_delay);  // 初始化 u8g2 结构体
    init_i2c_hw(&canvasBuffer, I2C_BUS);
    u8g2_SetI2CAddress(&canvasBuffer, I2C_ADDRESS);
    u8g2_InitDisplay(&canvasBuffer); // 根据所选的芯片进行初始化工作，初始化完成后，显示器处于关闭状态
    u8g2_SetPowerSave(&canvasBuffer, 0); // 打开显示器
    u8g2_ClearBuffer(&canvasBuffer);

    u8g2_SetFontMode(&canvasBuffer, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&canvasBuffer, 0); /*字体方向选择*/
    u8g2_SetFont(&canvasBuffer, astra::getUIConfig().mainFont); /*字库选择*/
    for(int i=0; i< key::KEY_NUM;i++){
        pinMode(HAL_GPIO_PINS[i], INPUT);
    }
}

bool Sh1106Hal::_getKey(key::KEY_INDEX _keyIndex) {
    bool clicked{false};
    clicked = digitalRead(HAL_GPIO_PINS[_keyIndex]) == HIGH;
    return clicked;
};

void Sh1106Hal::_screenOn() {};

void Sh1106Hal::_screenOff() {}