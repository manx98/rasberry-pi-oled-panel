#include "sh1106_hal/two_button_input.h"
#include <cassert>

using Action = TwoButtonInput::Action;

int main() {
    TwoButtonInput b;
    b.update(false, false, 0);
    // Contact bounce shorter than 20ms does not generate navigation.
    b.update(true, false, 10);
    b.update(false, false, 20);
    assert(b.takeAction() == Action::None);

    b.update(true, false, 100);
    b.update(true, false, 120);  // stable after 20ms
    assert(b.activeButton() == 0);
    assert(b.progress(445) > 0.49f && b.progress(445) < 0.51f);
    b.update(false, false, 470);
    b.update(false, false, 490);
    assert(b.takeAction() == Action::Previous);
    assert(b.takeAction() == Action::None);

    b.update(true, false, 500);
    b.update(true, false, 520);
    b.update(true, false, 1169);
    assert(b.takeAction() == Action::None);
    b.update(true, false, 1170);
    assert(b.takeAction() == Action::Confirm);
    b.update(false, false, 1200);
    b.update(false, false, 1220);
    assert(b.takeAction() == Action::None);

    b.update(false, true, 1400);
    b.update(false, true, 1420);
    b.update(false, false, 1450);
    b.update(false, false, 1470);
    assert(b.takeAction() == Action::Next);
    b.update(false, true, 1500);
    b.update(false, true, 1520);
    b.update(false, true, 2170);
    assert(b.takeAction() == Action::Cancel);
    b.update(false, false, 2300);
    b.update(false, false, 2320);
    assert(b.takeAction() == Action::None);

    // Presses recorded during a slow screen refresh remain queued in order.
    b.update(true, false, 2400);
    b.update(true, false, 2420);
    b.update(false, false, 2450);
    b.update(false, false, 2470);
    b.update(false, true, 2500);
    b.update(false, true, 2520);
    b.update(false, false, 2550);
    b.update(false, false, 2570);
    assert(b.takeAction() == Action::Previous);
    assert(b.takeAction() == Action::Next);
}
