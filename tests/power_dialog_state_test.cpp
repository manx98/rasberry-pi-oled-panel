#include "panel/power_dialog_state.h"
#include <cassert>
using State = PowerDialogState;
int main() {
    // Power menu has exactly two entries. Long Cancel closes directly.
    {
        State state;
        assert(state.selected() == 0); // Power off
        state.press(State::Key::Cancel);
        assert(state.finished());
        assert(state.confirmedAction() == State::Action::None);
    }
    // Both navigation directions wrap between shutdown and restart.
    {
        State state;
        state.press(State::Key::Previous);
        assert(state.selected() == 1); // Reboot
        state.press(State::Key::Next);
        assert(state.selected() == 0); // Power off
        state.press(State::Key::Next);
        assert(state.selected() == 1);
        state.press(State::Key::Confirm);
        assert(state.page() == State::Page::Confirmation);
        assert(state.requestedAction() == State::Action::Reboot);
        assert(state.selected() == 1); // Safe default: Return
        state.press(State::Key::Confirm);
        assert(state.page() == State::Page::Menu);
        assert(state.confirmedAction() == State::Action::None);
        state.press(State::Key::Confirm);
        state.press(State::Key::Previous); // Execute
        state.press(State::Key::Confirm);
        assert(state.finished());
        assert(state.confirmedAction() == State::Action::Reboot);
    }
    {
        State state;
        state.press(State::Key::Confirm); // Shutdown confirmation
        assert(state.requestedAction() == State::Action::PowerOff);
        state.press(State::Key::Cancel); // Back to power menu
        assert(state.page() == State::Page::Menu);
        assert(!state.finished());
        state.press(State::Key::Cancel); // Close popup
        assert(state.finished());
        assert(state.confirmedAction() == State::Action::None);
    }
    {
        State state;
        state.press(State::Key::Confirm);
        state.press(State::Key::Previous);
        state.press(State::Key::Confirm);
        assert(state.finished());
        assert(state.confirmedAction() == State::Action::PowerOff);
        state.press(State::Key::Confirm); // Ignore input after finishing
        assert(state.confirmedAction() == State::Action::PowerOff);
    }
}
