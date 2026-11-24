#pragma once

#include "sh1106_hal/two_button_input.h"

namespace display {

inline bool validRotation(int degrees) { return degrees == 0 || degrees == 180; }

// GPIO24 / GPIO23 are physically reversed when the OLED is mounted upside down.
// Keep the GPIO state machine in physical order and change only semantic events.
inline TwoButtonInput::Action mapAction(TwoButtonInput::Action physical, int degrees) {
    if (degrees != 180) return physical;
    switch (physical) {
        case TwoButtonInput::Action::Previous: return TwoButtonInput::Action::Next;
        case TwoButtonInput::Action::Next: return TwoButtonInput::Action::Previous;
        case TwoButtonInput::Action::Confirm: return TwoButtonInput::Action::Cancel;
        case TwoButtonInput::Action::Cancel: return TwoButtonInput::Action::Confirm;
        case TwoButtonInput::Action::None: return TwoButtonInput::Action::None;
    }
    return physical;
}

inline int logicalConfirmButton(int degrees) { return degrees == 180 ? 1 : 0; }
inline int logicalCancelButton(int degrees) { return degrees == 180 ? 0 : 1; }

} // namespace display
