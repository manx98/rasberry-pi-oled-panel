#include "panel/display/orientation_policy.h"
#include <cassert>

using Action = TwoButtonInput::Action;
int main() {
    assert(display::validRotation(0));
    assert(display::validRotation(180));
    assert(!display::validRotation(90));
    assert(!display::validRotation(270));
    for (Action action : {Action::None, Action::Previous, Action::Next, Action::Confirm, Action::Cancel}) {
        assert(display::mapAction(action, 0) == action);
        assert(display::mapAction(display::mapAction(action, 180), 180) == action);
    }
    assert(display::mapAction(Action::Previous, 180) == Action::Next);
    assert(display::mapAction(Action::Next, 180) == Action::Previous);
    assert(display::mapAction(Action::Confirm, 180) == Action::Cancel);
    assert(display::mapAction(Action::Cancel, 180) == Action::Confirm);
    assert(display::logicalConfirmButton(0) == 0);
    assert(display::logicalConfirmButton(180) == 1);
    assert(display::logicalCancelButton(180) == 0);
}
