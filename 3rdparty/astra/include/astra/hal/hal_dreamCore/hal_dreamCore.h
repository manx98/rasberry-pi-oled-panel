//
// Created by Fir on 2024/2/11.
//

#pragma once
#ifndef ASTRA_CORE_SRC_HAL_HAL_DREAMCORE_HAL_DREAMCORE_H_
#define ASTRA_CORE_SRC_HAL_HAL_DREAMCORE_HAL_DREAMCORE_H_

#include "astra/hal/hal.h"
#include <u8g2.h>
#include <random>

class HALDreamCore : public HAL {
public:
    HALDreamCore() = default;

protected:
    u8g2_t canvasBuffer{};

public:
    virtual inline void init() override {};

public:
    virtual void _screenOn() override {};

    virtual void _screenOff() override {};

public:
    void *_getCanvasBuffer() override;

    unsigned char _getBufferTileHeight() override;

    unsigned char _getBufferTileWidth() override;

    void _canvasUpdate() override;

    void _canvasClear() override;

    void _setFont(const unsigned char *_font) override;

    int _getFontWidth(const std::string &_text) override;

    unsigned char _getFontHeight() override;

    void _setDrawType(unsigned char _type) override;

    void _drawPixel(float _x, float _y) override;

    void _drawEnglish(float _x, float _y, const std::string &_text) override;

    void _drawChinese(float _x, float _y, const std::string &_text) override;

    void _drawVDottedLine(float _x, float _y, float _h) override;

    void _drawHDottedLine(float _x, float _y, float _l) override;

    void _drawVLine(float _x, float _y, float _h) override;

    void _drawHLine(float _x, float _y, float _l) override;

    void _drawBMP(float _x, float _y, float _w, float _h, const unsigned char *_bitMap) override;

    void _drawBox(float _x, float _y, float _w, float _h) override;

    void _drawRBox(float _x, float _y, float _w, float _h, float _r) override;

    void _drawFrame(float _x, float _y, float _w, float _h) override;

    void _drawRFrame(float _x, float _y, float _w, float _h, float _r) override;

public:
    virtual void _delay(unsigned long _mill) override;

    virtual int64_t _millis() override;

    virtual unsigned long _getTick() override;

    virtual unsigned long _getRandomSeed() override;

public:
    void _beep(float _freq) override;

    void _beepStop() override;

    void _setBeepVol(unsigned char _vol) override;

public:
    bool _getKey(key::KEY_INDEX _keyIndex) override {
        return false;
    };
public:
    void _updateConfig() override;

private:
    std::random_device rd_;
};

#endif //ASTRA_CORE_SRC_HAL_HAL_DREAMCORE_HAL_DREAMCORE_H_
