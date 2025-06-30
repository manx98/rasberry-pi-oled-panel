//
// Created by wenyiyu on 2025/6/28.
//

#ifndef SDL_HAL_H
#define SDL_HAL_H
#include "astra/hal/hal_dreamCore/hal_dreamCore.h"
class SDLHal: public HALDreamCore {
public:
    bool _getKey(key::KEY_INDEX _keyIndex) override;
    void _screenOn() override;
    void _screenOff() override;
    void init() override;
};



#endif //SDL_HAL_H
