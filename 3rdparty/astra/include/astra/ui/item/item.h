//
// Created by Fir on 2024/4/14 014.
//
#pragma once
#ifndef ASTRA_CORE_SRC_ASTRA_UI_ITEM_ITEM_H_
#define ASTRA_CORE_SRC_ASTRA_UI_ITEM_ITEM_H_

#include <cmath>
#include "astra/hal/hal.h"
#include "astra/config/config.h"
#include "astra/utils/clocker.h"

namespace astra
{
    class Item
    {
    protected:
        sys::config systemConfig;
        config astraConfig;

        inline void updateConfig()
        {
            this->systemConfig = HAL::getSystemConfig();
            this->astraConfig = getUIConfig();
        }
    };

    class Animation
    {
    public:
        static void entry();

        static void exit();

        static void blur();

        static void move(float& _pos, float _posTrg, float _speed, Clocker& clocker);

        static void moveUniform(float& _pos, float _posTrg, float _speed, Clocker& clocker);
    };

    inline void Animation::entry()
    {
    }

    inline void Animation::blur()
    {
        static unsigned char bufferLen = 8 * HAL::getBufferTileHeight() * HAL::getBufferTileWidth();
        static auto* bufferPointer = (unsigned char*)HAL::getCanvasBuffer();

        for (uint16_t i = 0; i < bufferLen; ++i) bufferPointer[i] = bufferPointer[i] & (i % 2 == 0 ? 0x55 : 0xAA);
    }

    inline void Animation::move(float& _pos, float _posTrg, float _speed, Clocker& clocker)
    {
        auto update_times = clocker.lastDuration() / getUIConfig().perFrameMills;
        if (update_times < 1)
        {
            update_times = 1;
        }
        for (; update_times > 0 && _pos != _posTrg; update_times--)
        {
            if (std::fabs(_pos - _posTrg) <= 1.0f)
            {
                _pos = _posTrg;
            }
            else
            {
                _pos += (_posTrg - _pos) / ((100 - _speed) / 1.0f);
            }
        }
    }

    inline void Animation::moveUniform(float& _pos, float _posTrg, float _speed, Clocker& clocker)
    {
        _pos += clocker.lastDuration() / _speed;
        if (_pos >= _posTrg)
        {
            _pos = _posTrg;
        }
    }
}

#endif //ASTRA_CORE_SRC_ASTRA_UI_ITEM_ITEM_H_
