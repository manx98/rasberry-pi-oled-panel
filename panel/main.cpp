#include <unistd.h>

#include "astra_rocket.h"
#include "sh1106_hal/sdl_hal.h"

int main() {
    astraCoreInit();
    astraCoreStart();
}
