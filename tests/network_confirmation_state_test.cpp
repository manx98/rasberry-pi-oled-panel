#include "panel/network_confirmation_state.h"
#include <cassert>
using State = NetworkConfirmationState;

int main() {
    {
        State dialog;
        assert(dialog.selected() == 1); // Initial focus: Cancel
        assert(!dialog.finished());
        assert(!dialog.confirmed());
        dialog.press(State::Key::Confirm);
        assert(dialog.finished());
        assert(!dialog.confirmed());
    }
    {
        State dialog;
        dialog.press(State::Key::Cancel);
        assert(dialog.finished());
        assert(!dialog.confirmed());
    }
    {
        State dialog;
        dialog.press(State::Key::Previous);
        assert(dialog.selected() == 0);
        dialog.press(State::Key::Confirm);
        assert(dialog.finished());
        assert(dialog.confirmed());
        dialog.press(State::Key::Next); // Ignore events after completion
        assert(dialog.confirmed());
    }
    {
        State dialog;
        dialog.press(State::Key::Next);
        assert(dialog.selected() == 0);
        dialog.press(State::Key::Cancel);
        assert(!dialog.confirmed());
    }
    {
        State dialog;
        dialog.press(State::Key::Previous);
        dialog.press(State::Key::Previous);
        assert(dialog.selected() == 1);
        dialog.press(State::Key::Confirm);
        assert(!dialog.confirmed());
    }
}
