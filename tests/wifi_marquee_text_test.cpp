#include "astra/ui/item/widget/widget.h"
#include "astra/hal/hal.h"
#include "astra/config/config.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>

// Test-only font placeholders satisfy Astra's UI configuration defaults.
// Fake HAL never interprets or renders the font bytes.
extern "C" {
extern const unsigned char u8g2_font_open_iconic_arrow_1x_t[] = {0};
extern const unsigned char u8g2_font_boutique_bitmap_7x7_t_gb2312[] = {0};
extern const unsigned char u8g2_font_boutique_bitmap_9x9_t_gb2312[] = {0};
}

// Small HAL stub to test real Astra text clipping/animation without touching
// the OLED, Wi-Fi hardware, or any live NetworkManager connections.
struct MarqueeTestHal final : HAL {
    int64_t clockMs = 0;
    float drawX = -1;
    float clipLeft = -1;
    float clipRight = -1;
    int clips = 0;
    int clipRestores = 0;
    void _setFont(const unsigned char *) override {}
    float _getFontWidth(const std::string &s) override {
        return static_cast<float>(s.size()) * 8;
    }
    unsigned char _getFontHeight() override { return 9; }
    int64_t _millis() override { return clockMs; }
    void _drawChinese(float x, float, const std::string &) override { drawX = x; }
    void _setClipWindow(float left, float, float right, float) override {
        clipLeft = left;
        clipRight = right;
        ++clips;
    }
    void _setMaxClipWindow() override { ++clipRestores; }
};

int main() {
    auto *hal = new MarqueeTestHal();
    assert(HAL::inject(hal));
    astra::getUIConfig().text_flow_speed = 25;
    astra::getUIConfig().text_flow_boundary_wait_time = 500;
    astra::Clocker clock;

    astra::TextBox longLabel{0, {{"[92] [locked] LongGuestNetworkName", nullptr}}};
    longLabel.setMaxWidth(105);
    assert(longLabel.getWidth() == 105);
    auto frame = [&](bool allowScroll, int advance) {
        hal->clockMs += advance;
        clock.next();
        longLabel.draw(4, 24, clock, allowScroll);
        assert(hal->clipLeft == 4 && hal->clipRight == 109);
        assert(hal->clipRestores == hal->clips);
        return hal->drawX;
    };

    // Long unselected labels are clipped, but never animated.
    for (int n = 0; n < 25; ++n) assert(frame(false, 100) == 4);
    // Selected labels show their beginning first, then scroll after a dwell.
    // The initial pause is 500 ms. The fifth 100-ms frame starts motion.
    assert(frame(true, 100) == 4);
    assert(frame(true, 100) == 4);
    assert(frame(true, 100) == 4);
    assert(frame(true, 100) == 4);
    assert(frame(true, 100) < 4);
    // Changing selection resets a label to its beginning.
    assert(frame(false, 100) == 4);
    assert(frame(true, 100) == 4);

    // Short SSIDs display normally without clipping or animation.
    astra::TextBox shortLabel{0, {{"Short", nullptr}}};
    shortLabel.setMaxWidth(105);
    assert(shortLabel.getWidth() == 40);
    const int clipsBefore = hal->clips;
    shortLabel.draw(4, 24, clock, true);
    assert(hal->drawX == 4);
    assert(hal->clips == clipsBefore);
    HAL::destroy();
}
