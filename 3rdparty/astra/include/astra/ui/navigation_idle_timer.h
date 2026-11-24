#pragma once

#include <cstdint>

namespace astra {

// Deterministic inactivity timer: resets on page change or any key action.
// Only pages explicitly enabling this feature may hide navigation chrome.
class NavigationIdleTimer {
public:
    static constexpr std::uint64_t defaultTimeoutMs = 30000;

    // A timeout of zero explicitly disables hiding for that page.
    bool shouldHide(const void *page, bool enabled, bool userAction,
                    std::uint64_t nowMs, std::uint64_t timeoutMs = defaultTimeoutMs) {
        if (!initialized_ || page != page_) {
            page_ = page;
            lastActivityMs_ = nowMs;
            initialized_ = true;
        }
        if (userAction || !enabled || timeoutMs == 0) lastActivityMs_ = nowMs;
        return enabled && timeoutMs != 0 && nowMs >= lastActivityMs_
               && nowMs - lastActivityMs_ >= timeoutMs;
    }

private:
    const void *page_ = nullptr;
    std::uint64_t lastActivityMs_ = 0;
    bool initialized_ = false;
};

} // namespace astra
