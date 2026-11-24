#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>

// Pure input state machine, independent of GPIO and UI; timestamps are monotonic ms.
class TwoButtonInput {
public:
    enum class Action { None, Previous, Next, Confirm, Cancel };
    static constexpr uint64_t debounceMs = 20;
    static constexpr uint64_t holdMs = 650;

    void update(bool gpio24, bool gpio23, uint64_t now) {
        const bool raw[2] = {gpio24, gpio23};
        for (int i = 0; i < 2; ++i) {
            auto &b = buttons_[i];
            if (raw[i] != b.raw) {
                b.raw = raw[i];
                b.changedAt = now;
            }
            if (b.stable != b.raw && now - b.changedAt >= debounceMs) {
                b.stable = b.raw;
                if (b.stable) {
                    b.pressedAt = now;
                    b.longSent = false;
                } else if (!b.longSent) {
                    actions_.push_back(i == 0 ? Action::Previous : Action::Next);
                }
            }
            if (b.stable && !b.longSent && now - b.pressedAt >= holdMs) {
                b.longSent = true;
                actions_.push_back(i == 0 ? Action::Confirm : Action::Cancel);
            }
        }
    }

    Action takeAction() {
        if (actions_.empty()) return Action::None;
        const Action result = actions_.front();
        actions_.pop_front();
        return result;
    }

    // 0 = GPIO24/confirm, 1 = GPIO23/cancel. -1 = no active hold.
    int activeButton() const {
        for (int i = 0; i < 2; ++i) {
            if (buttons_[i].stable) return i;
        }
        return -1;
    }

    float progress(uint64_t now) const {
        const int i = activeButton();
        if (i < 0) return 0.0f;
        const auto &b = buttons_[i];
        return b.longSent ? 1.0f : static_cast<float>(std::min(holdMs, now - b.pressedAt)) / holdMs;
    }

private:
    struct Button {
        bool raw = false;
        bool stable = false;
        bool longSent = false;
        uint64_t changedAt = 0;
        uint64_t pressedAt = 0;
    };
    std::array<Button, 2> buttons_{};
    std::deque<Action> actions_{};
};
