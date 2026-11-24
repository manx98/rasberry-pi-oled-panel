#pragma once

#include <astra/hal/hal_dreamCore/hal_dreamCore.h>
#include "sh1106_hal/two_button_input.h"
#include <array>
#include <atomic>
#include <mutex>
#include <thread>

// Standalone cleanup; does not claim button GPIO or initialize the UI.
int clearSh1106Display();

class Sh1106Hal final : public HALDreamCore {
public:
    ~Sh1106Hal() override;
    int shutdown() noexcept;
    void init() override;
    bool _keyScan() override;
    bool _getKey(key::KEY_INDEX index) override;
    void _canvasUpdate() override;
    void _screenOn() override;
    void _screenOff() override;

private:
    void scanButtons();

    int lineFd_ = -1;
    std::atomic<bool> scanning_{false};
    std::thread buttonThread_;
    std::mutex buttonsMutex_;
    TwoButtonInput buttons_;
    std::array<unsigned char, 1024> sentBuffer_{};
    bool hasSentFrame_ = false;
    bool displayInitialized_ = false;
    int appliedRotationDegrees_ = 0;
};
