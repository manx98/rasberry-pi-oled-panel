//
// Created by wenyiyu on 2025/6/28.
//

#include "sdl_hal.h"
#include "SDL.h"
#include "common/log.h"

static bool KEY_DOWN[key::KEY_NUM];

static void proceedKey(bool is_down, SDL_Keycode code)
{
    switch (code)
    {
    case SDLK_ESCAPE:
        KEY_DOWN[key::KEY_CANCEL] = is_down;
        break;
    case SDLK_RETURN:
        KEY_DOWN[key::KEY_CONFIRM] = is_down;
        break;
    case SDLK_RIGHT:
    case SDLK_DOWN:
        KEY_DOWN[key::KEY_NEXT] = is_down;
        break;
    case SDLK_LEFT:
    case SDLK_UP:
        KEY_DOWN[key::KEY_PREV] = is_down;
        break;
    }
}

bool SDLHal::_getKey(key::KEY_INDEX _keyIndex)
{
    SDL_Event event;
    key::KEY_INDEX current = key::KEY_NUM;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
        case SDL_KEYDOWN:
            proceedKey(true, event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            proceedKey(false, event.key.keysym.sym);
            break;
        default:
            ;
        }
    }
    return KEY_DOWN[_keyIndex];
}

void SDLHal::_screenOn()
{
}

void SDLHal::_screenOff()
{
}

void SDLHal::init()
{
    HALDreamCore::init();
    u8g2_SetupBuffer_SDL_128x64(&canvasBuffer, &u8g2_cb_r0);
    u8x8_InitDisplay(u8g2_GetU8x8(&canvasBuffer));
    u8x8_SetPowerSave(u8g2_GetU8x8(&canvasBuffer), 0);
    u8g2_ClearBuffer(&canvasBuffer);

    u8g2_SetFontMode(&canvasBuffer, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&canvasBuffer, 0); /*字体方向选择*/
    u8g2_SetFont(&canvasBuffer, astra::getUIConfig().mainFont); /*字库选择*/
}
