#include <unistd.h>

#include "astra_rocket.h"
#include "sh1106_hal/sdl_hal.h"

int main() {
    astraCoreInit();
    astraCoreStart();
    // auto hal = new SDLHal;
    // HAL::inject(hal);
    // auto width = HAL::getFontWidth("Hello World!");
    // int current = 0;
    // bool is_forward = false;
    // while (true) {
    //     hal->canvasClear();
    //     u8g2_DrawUTF8(hal->getBuffer(), current, 40, "Hello World!");
    //     u8g2_SetClipWindow(hal->getBuffer(), 20, 0, 128, 64);
    //     u8g2_DrawUTF8(hal->getBuffer(), current, 20, "Hello World!");
    //     u8g2_SetMaxClipWindow(hal->getBuffer());
    //     HAL::canvasUpdate();
    //     if (is_forward)
    //     {
    //         current += 1;
    //         if (current >= 0)
    //         {
    //             is_forward = !is_forward;
    //         }
    //     } else
    //     {
    //         current -= 1;
    //         if (current <= -width)
    //         {
    //             is_forward = !is_forward;
    //         }
    //     }
    //     HAL::getKey(key::KEY_PREV);
    //     usleep(50000);
    // }
}
