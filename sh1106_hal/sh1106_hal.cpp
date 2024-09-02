//
// Created by wenyiyu on 2024/9/1.
//

#include "sh1106_hal.h"
#include <u8g2port.h>

// Set I2C bus and address
#define I2C_BUS 1
#define I2C_ADDRESS 0x3c * 2
#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GPIO_KEY_0 25
#define GPIO_KEY_1 26
#define GPIO_PRESS_KEY 16

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
    u8g2_SetFont(&canvasBuffer, u8g2_font_myfont); /*字库选择*/

    key0_ = gpio_new();
    if (gpio_open(key0_, GPIO_CHIP_PATH, GPIO_KEY_0, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(key0_));
        exit(1);
    }

    key1_ = gpio_new();
    if (gpio_open(key1_, GPIO_CHIP_PATH, GPIO_KEY_1, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(key1_));
        exit(1);
    }

    press_key_ = gpio_new();
    if (gpio_open(press_key_, GPIO_CHIP_PATH, GPIO_PRESS_KEY, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(press_key_));
        exit(1);
    }
}

bool Sh1106Hal::_getKey(key::KEY_INDEX _keyIndex) {
    bool clicked{false};
    gpio_read(press_key_, &clicked);
    if (clicked) {
        return true;
    } else if (_keyIndex == key::KEY_0) {
        gpio_read(key0_, &clicked);
    } else if (_keyIndex == key::KEY_1) {
        gpio_read(key1_, &clicked);
    }
    return clicked;
};

void Sh1106Hal::_screenOn() {};

void Sh1106Hal::_screenOff() {}