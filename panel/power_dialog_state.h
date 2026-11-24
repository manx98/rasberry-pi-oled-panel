#pragma once

// Small, testable state machine for the root-screen power dialog.
// Deliberately contains no GPIO, OLED or system-power dependencies.
class PowerDialogState {
public:
    enum class Key { Previous, Next, Confirm, Cancel };
    enum class Action { None, PowerOff, Reboot };
    enum class Page { Menu, Confirmation, Closed };

    Page page() const { return page_; }
    int selected() const { return page_ == Page::Confirmation ? confirmIndex_ : menuIndex_; }
    Action requestedAction() const { return requestedAction_; }
    Action confirmedAction() const { return confirmedAction_; }
    bool finished() const { return page_ == Page::Closed; }

    void press(Key key) {
        if (finished()) return;
        if (key == Key::Cancel) {
            if (page_ == Page::Confirmation) {
                page_ = Page::Menu;
                confirmIndex_ = 1;
            } else {
                page_ = Page::Closed;
            }
            return;
        }
        if (page_ == Page::Menu) {
            if (key == Key::Previous) menuIndex_ = (menuIndex_ + 1) % 2;
            else if (key == Key::Next) menuIndex_ = (menuIndex_ + 1) % 2;
            else if (key == Key::Confirm) {
                requestedAction_ = menuIndex_ == 0 ? Action::PowerOff : Action::Reboot;
                confirmIndex_ = 1; // Always default to 'return', not execute.
                page_ = Page::Confirmation;
            }
        } else if (page_ == Page::Confirmation) {
            if (key == Key::Previous || key == Key::Next) confirmIndex_ = 1 - confirmIndex_;
            else if (key == Key::Confirm) {
                if (confirmIndex_ == 0) {
                    confirmedAction_ = requestedAction_;
                    page_ = Page::Closed;
                } else {
                    requestedAction_ = Action::None;
                    page_ = Page::Menu;
                }
            }
        }
    }

private:
    Page page_ = Page::Menu;
    int menuIndex_ = 0;      // Only shutdown/reboot; confirmation defaults to safe return.
    int confirmIndex_ = 1;   // Focus 'return' when confirming a power action.
    Action requestedAction_ = Action::None;
    Action confirmedAction_ = Action::None;
};
