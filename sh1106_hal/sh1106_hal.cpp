#include "sh1106_hal.h"
#include "sh1106_hal/clear_display.h"
#include "panel/display/orientation_config.h"
#include "panel/display/orientation_policy.h"

#include <u8g2port.h>
#include <linux/gpio.h>
#include <linux/i2c-dev.h>
#include <cerrno>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <system_error>

namespace {
constexpr int i2cBus = 1;
constexpr int i2cAddress = 0x3c * 2;

uint64_t monotonicMs() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

int requestButtons() {
    const int chip = open("/dev/gpiochip0", O_RDONLY | O_CLOEXEC);
    if (chip < 0) throw std::system_error(errno, std::generic_category(), "open /dev/gpiochip0");
    gpio_v2_line_request request{};
    request.offsets[0] = 24;
    request.offsets[1] = 23;
    request.num_lines = 2;
    std::strncpy(request.consumer, "oled-panel-buttons", sizeof(request.consumer) - 1);
    request.config.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
    const int err = ioctl(chip, GPIO_V2_GET_LINE_IOCTL, &request);
    const int savedErrno = errno;
    close(chip);
    if (err < 0) throw std::system_error(savedErrno, std::generic_category(), "request GPIO24/23 inputs");
    return request.fd;
}
}

Sh1106Hal::~Sh1106Hal() {
    shutdown();
}

int Sh1106Hal::shutdown() noexcept {
    scanning_ = false;
    if (buttonThread_.joinable()) buttonThread_.join();
    int status = 0;
    if (displayInitialized_) {
        // No more UI rendering or button overlays can race this full clear.
        status = clearSh1106Display();
        done_user_data(&canvasBuffer);
        done_i2c();
        displayInitialized_ = false;
        if (status == 0) std::fprintf(stderr, "OLED display cleared on shutdown\n");
    }
    if (lineFd_ >= 0) { close(lineFd_); lineFd_ = -1; }
    return status;
}

int clearSh1106Display() {
    const int fd = open("/dev/i2c-1", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        std::perror("open OLED I2C bus for cleanup");
        return 1;
    }
    if (ioctl(fd, I2C_SLAVE, i2cAddress >> 1) < 0) {
        std::perror("select OLED I2C address for cleanup");
        close(fd);
        return 1;
    }
    const bool ok = sh1106::clearDisplay([fd](const uint8_t *data, std::size_t size) {
        ssize_t count;
        do { count = write(fd, data, size); } while (count < 0 && errno == EINTR);
        if (count == static_cast<ssize_t>(size)) return true;
        if (count >= 0) errno = EIO;
        std::perror("clear OLED I2C write");
        return false;
    });
    close(fd);
    return ok ? 0 : 1;
}

void Sh1106Hal::init() {
    HALDreamCore::init();
    // Reserve input lines first, so failed GPIO configuration does not silently
    // accept ghost presses. Bias pulldown: buttons connect GPIO to 3.3V.
    lineFd_ = requestButtons();
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&canvasBuffer, U8G2_R0,
                                          u8x8_byte_arm_linux_hw_i2c,
                                          u8x8_arm_linux_gpio_and_delay);
    init_i2c_hw(&canvasBuffer, i2cBus);
    displayInitialized_ = true;
    u8g2_SetI2CAddress(&canvasBuffer, i2cAddress);
    u8g2_InitDisplay(&canvasBuffer);
    // SH1106 supports 180-degree flip in hardware (segment remap + COM scan
    // direction). Unlike rotating the draw buffer, this also rotates the
    // long-press overlay and preserves the fast dirty-tile update format.
    appliedRotationDegrees_ = display::OrientationConfig::instance().degrees();
    u8g2_SetFlipMode(&canvasBuffer, appliedRotationDegrees_ == 180 ? 1 : 0);
    u8g2_SetPowerSave(&canvasBuffer, 0);
    u8g2_ClearBuffer(&canvasBuffer);
    u8g2_SetFontMode(&canvasBuffer, 1);
    u8g2_SetFontDirection(&canvasBuffer, 0);
    u8g2_SetFont(&canvasBuffer, astra::getUIConfig().mainFont);
    scanning_ = true;
    try {
        buttonThread_ = std::thread(&Sh1106Hal::scanButtons, this);
    } catch (...) {
        scanning_ = false;
        throw;
    }
}

void Sh1106Hal::scanButtons() {
    // GPIO is sampled independently of the OLED's blocking I2C writes.
    // A 5ms interval plus 20ms debounce also catches short taps while the
    // UI thread is spending time on screen rendering or NetworkManager calls.
    bool previous24 = false;
    bool previous23 = false;
    bool reportedReadError = false;
    while (scanning_) {
        gpio_v2_line_values values{};
        values.mask = 0x3;
        if (ioctl(lineFd_, GPIO_V2_LINE_GET_VALUES_IOCTL, &values) == 0) {
            previous24 = (values.bits & 1) != 0;
            previous23 = (values.bits & 2) != 0;
            std::lock_guard<std::mutex> lock(buttonsMutex_);
            buttons_.update(previous24, previous23, monotonicMs());
            reportedReadError = false;
        } else if (!reportedReadError) {
            std::perror("read GPIO24/23");
            reportedReadError = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool Sh1106Hal::_getKey(key::KEY_INDEX index) {
    std::lock_guard<std::mutex> lock(buttonsMutex_);
    const int active = buttons_.activeButton();
    const bool reversed = display::OrientationConfig::instance().degrees() == 180;
    return (index == key::KEY_PREV && active == (reversed ? 1 : 0)) ||
           (index == key::KEY_NEXT && active == (reversed ? 0 : 1));
}

bool Sh1106Hal::_keyScan() {
    TwoButtonInput::Action action;
    {
        std::lock_guard<std::mutex> lock(buttonsMutex_);
        action = buttons_.takeAction();
    }
    // Both short and long-press meanings reverse at 180 degrees.
    action = display::mapAction(action, display::OrientationConfig::instance().degrees());
    for (auto &entry : key) entry = key::INVALID;
    switch (action) {
        case TwoButtonInput::Action::Previous: key[key::KEY_PREV] = key::CLICK; break;
        case TwoButtonInput::Action::Next: key[key::KEY_NEXT] = key::CLICK; break;
        case TwoButtonInput::Action::Confirm: key[key::KEY_CONFIRM] = key::CLICK; break;
        case TwoButtonInput::Action::Cancel: key[key::KEY_CANCEL] = key::CLICK; break;
        case TwoButtonInput::Action::None: return false;
    }
    return true;
}

void Sh1106Hal::_canvasUpdate() {
    // The setting can be changed live, with no GPIO reinitialization or app
    // restart. The SH1106 rotates the physical display through two commands.
    const int requestedRotation = display::OrientationConfig::instance().degrees();
    if (requestedRotation != appliedRotationDegrees_) {
        u8g2_SetFlipMode(&canvasBuffer, requestedRotation == 180 ? 1 : 0);
        appliedRotationDegrees_ = requestedRotation;
        hasSentFrame_ = false; // Ensure a full frame is sent after rotation.
    }
    // Draw over the regular UI every frame; no blocking modal while holding.
    int active;
    float fraction;
    {
        std::lock_guard<std::mutex> lock(buttonsMutex_);
        active = buttons_.activeButton();
        fraction = buttons_.progress(monotonicMs());
    }
    if (active >= 0) {
        u8g2_SetDrawColor(&canvasBuffer, 0);
        u8g2_DrawBox(&canvasBuffer, 0, 51, 128, 13);
        u8g2_SetDrawColor(&canvasBuffer, 1);
        u8g2_SetFont(&canvasBuffer, u8g2_font_5x7_tr);
        const bool confirm = active == display::logicalConfirmButton(requestedRotation);
        u8g2_DrawStr(&canvasBuffer, 2, 60, confirm ? "OK" : "BACK");
        u8g2_DrawFrame(&canvasBuffer, 34, 54, 91, 8);
        const auto width = static_cast<u8g2_uint_t>(87 * fraction);
        if (width > 0) u8g2_DrawBox(&canvasBuffer, 36, 56, width, 4);
        u8g2_SetFont(&canvasBuffer, astra::getUIConfig().mainFont);
    }
    // SH1106 I2C writes are expensive (1024 bytes for a full frame).
    // Only transfer 8x8 tiles whose contents changed since the last frame.
    // Progress animation typically changes only the bottom 1-2 tile rows.
    const auto *frame = u8g2_GetBufferPtr(&canvasBuffer);
    if (!hasSentFrame_) {
        u8g2_SendBuffer(&canvasBuffer);
        std::memcpy(sentBuffer_.data(), frame, sentBuffer_.size());
        hasSentFrame_ = true;
        return;
    }

    struct DirtyRow { int minX = 16; int maxX = -1; } rows[8];
    int totalDirtyTiles = 0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 16; ++x) {
            const int offset = y * 128 + x * 8;
            if (std::memcmp(frame + offset, sentBuffer_.data() + offset, 8) != 0) {
                rows[y].minX = std::min(rows[y].minX, x);
                rows[y].maxX = x;
                ++totalDirtyTiles;
            }
        }
    }
    if (totalDirtyTiles == 0) return;
    if (totalDirtyTiles > 90) {
        u8g2_SendBuffer(&canvasBuffer);
    } else {
        for (int y = 0; y < 8; ++y) {
            if (rows[y].maxX >= rows[y].minX) {
                u8g2_UpdateDisplayArea(&canvasBuffer, rows[y].minX, y,
                                        rows[y].maxX - rows[y].minX + 1, 1);
            }
        }
    }
    std::memcpy(sentBuffer_.data(), frame, sentBuffer_.size());
}

void Sh1106Hal::_screenOn() {
    u8g2_SetPowerSave(&canvasBuffer, 0);
    hasSentFrame_ = false;
}
void Sh1106Hal::_screenOff() {
    u8g2_SetPowerSave(&canvasBuffer, 1);
    hasSentFrame_ = false;
}
