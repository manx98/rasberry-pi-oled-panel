#pragma once

// The network interface is never modified until this two-key dialog
// explicitly confirms execution. Default choice is Cancel.
class NetworkConfirmationState {
public:
    enum class Key { Previous, Next, Confirm, Cancel };

    bool finished() const { return finished_; }
    bool confirmed() const { return finished_ && selected_ == 0 && accepted_; }
    int selected() const { return selected_; }

    void press(Key key) {
        if (finished_) return;
        switch (key) {
            case Key::Previous:
            case Key::Next:
                selected_ = 1 - selected_;
                break;
            case Key::Confirm:
                accepted_ = selected_ == 0;
                finished_ = true;
                break;
            case Key::Cancel:
                accepted_ = false;
                finished_ = true;
                break;
        }
    }

private:
    int selected_ = 1; // Cancel, never Execute on initial dialog open.
    bool accepted_ = false;
    bool finished_ = false;
};
