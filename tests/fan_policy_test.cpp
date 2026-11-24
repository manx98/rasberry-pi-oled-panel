#include "panel/fan/fan_policy.h"
#include <cassert>
using namespace fan;
int main() {
    Config c{};
    assert(valid(c));
    assert(c.mode == Mode::Auto && c.onTemp == 55 && c.offTemp == 45);
    assert(!outputFor(c, false, true, 54.9));
    assert(outputFor(c, false, true, 55));
    assert(outputFor(c, true, true, 46));
    assert(!outputFor(c, true, true, 45));
    assert(outputFor(c, false, false, 0)); // sensor failure turns fan ON
    c.mode = Mode::AlwaysOn;
    assert(outputFor(c, false, true, 20));
    c.mode = Mode::Off;
    assert(!outputFor(c, true, false, 0));
    c.mode = Mode::Auto;
    assert(gpioHighFor(true, true));
    assert(!gpioHighFor(false, true));
    assert(!gpioHighFor(true, false));
    c.offTemp = 54;
    assert(!valid(c));
    c.onTemp = 56;
    assert(valid(c));
    c.onTemp = 90;
    assert(!valid(c));
    c.onTemp = 50; c.offTemp = 20;
    assert(!valid(c));
}
