//
// Created by wenyiyu on 2025/7/5.
//

#include "astra/ui/item/item.h"

#include <astra/ui/launcher.h>

void astra::Animation::exit()
    {
        char fadeFlag = 1;
        static int bufferLen = 8 * HAL::getBufferTileHeight() * HAL::getBufferTileWidth();
        auto* bufferPointer = (unsigned char*)HAL::getCanvasBuffer();
        int64_t duration = 0;
        while (fadeFlag)
        {
            Launcher::update(false, [&](Clocker& clocker, key::keyIndex active_key)
            {
                if (fadeFlag != 1)
                {
                    duration += clocker.lastDuration();
                    if (duration >= getUIConfig().fadeAnimationSpeed)
                    {
                        while (duration >= getUIConfig().fadeAnimationSpeed)
                        {
                            duration -= getUIConfig().fadeAnimationSpeed;
                            fadeFlag += 1;
                        }
                        if (fadeFlag >= 5)
                        {
                            fadeFlag = 0;
                            return;
                        }
                    } else
                    {
                        return;
                    }

                }
                if (getUIConfig().lightMode)
                    switch (fadeFlag)
                    {
                    case 1:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 != 0)
                                bufferPointer[i] = bufferPointer[i] &
                                    0xAA;
                        fadeFlag = 2;
                        break;
                    case 2:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 != 0)
                                bufferPointer[i] = bufferPointer[i] &
                                    0x00;
                        break;
                    case 3:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 == 0)
                                bufferPointer[i] = bufferPointer[i] &
                                    0x55;
                        break;
                    case 4:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 == 0)
                                bufferPointer[i] = bufferPointer[i] &
                                    0x00;
                        break;
                    default:
                        //放动画结束退出函数的代码
                        fadeFlag = 0;
                        break;
                    }
                else
                    switch (fadeFlag)
                    {
                    case 1:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 != 0)
                                bufferPointer[i] = bufferPointer[i] |
                                    0xAA;
                        fadeFlag = 2;
                        break;
                    case 2:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 != 0)
                                bufferPointer[i] = bufferPointer[i] |
                                    0x00;
                        break;
                    case 3:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 == 0)
                                bufferPointer[i] = bufferPointer[i] |
                                    0x55;
                        break;
                    case 4:
                        for (uint16_t i = 0; i < bufferLen; ++i)
                            if (i % 2 == 0)
                                bufferPointer[i] = bufferPointer[i] |
                                    0x00;
                        break;
                    default:
                        fadeFlag = 0;
                        break;
                    }
            });
        }
    }
