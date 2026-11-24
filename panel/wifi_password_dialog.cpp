#include "panel/wifi_password_dialog.h"
#include "panel/wifi_password_state.h"
#include "panel/network_confirmation_state.h"
#include "astra/ui/launcher.h"
#include "astra/hal/hal.h"
#include <string>
#include <algorithm>

namespace {

// Truncate on UTF-8 character boundaries, not in the middle of Chinese SSIDs.
std::string fitText(std::string value, float maxWidth) {
    while (!value.empty() && HAL::getFontWidth(value) > maxWidth) {
        std::size_t start = value.size() - 1;
        while (start > 0 && (static_cast<unsigned char>(value[start]) & 0xC0u) == 0x80u)
            --start;
        value.erase(start); // Remove the full last UTF-8 codepoint.
    }
    return value;
}

// Draw a focus indication without hiding glyphs: U8g2 XOR inverts the
// already drawn text. Works in both hardware screen orientations.
void highlight(float x, float y, float width, float height) {
    HAL::setDrawType(2);
    HAL::drawRBox(x, y, width, height, 2);
    HAL::setDrawType(1);
}

void drawCharacterCarousel(const WifiPasswordState &input) {
    // Five visible characters: previous two, previous one, selected, next
    // one, next two. The chosen middle character is clearly inverted.
    for (int offset = -2; offset <= 2; ++offset) {
        const char chosen = input.neighboringCharacter(offset);
        const std::string glyph(1, chosen);
        const float x = 62 + offset * 20 - HAL::getFontWidth(glyph) / 2;
        // U8g2 uses a text baseline, not a top-left glyph coordinate.
        // Raise all five glyphs by 3px to visually center the selected
        // character within the unchanged y=37..50 highlight background.
        HAL::drawEnglish(x, 46, glyph);
    }
    highlight(52, 37, 20, 14);
    const std::string text = std::string(WifiPasswordState::categoryName(input.category()))
        + " " + std::to_string(input.characterPosition() + 1)
        + "/" + std::to_string(WifiPasswordState::characters(input.category()).size());
    HAL::drawChinese(8, 60, fitText(input.error().empty() ? text : input.error(), 111));
}

// The password editing viewport scrolls one character at a time, keeping
// the focused glyph visible even for a 63-character passphrase.
void drawEditPassword(const WifiPasswordState &input) {
    HAL::setDrawType(1);
    HAL::drawRFrame(2, 1, 124, 62, 3);
    HAL::setFont(astra::getUIConfig().mainFont);
    HAL::drawChinese(7, 11, "删除字符 (明文)");
    const auto &password = input.editPassword();
    const auto size = password.size();
    const auto pos = input.editPosition();
    HAL::drawChinese(7, 23,
        input.editSelectedCharacter()
            ? "位置 " + std::to_string(pos + 1) + "/" + std::to_string(size)
            : "长度 " + std::to_string(size) + " 位");

    HAL::drawRFrame(4, 25, 120, 15, 2);
    const float step = std::max(9.0f, HAL::getFontWidth("W") + 3.0f);
    const std::size_t visible = std::max<std::size_t>(1, static_cast<std::size_t>(94.0f / step));
    const std::size_t focused = pos < size ? pos : (size ? size - 1 : 0);
    const std::size_t start = size <= visible ? 0
        : std::min(focused > visible / 2 ? focused - visible / 2 : 0, size - visible);
    const std::size_t end = std::min(size, start + visible);
    if (start) HAL::drawEnglish(6, 37, "<");
    if (end < size) HAL::drawEnglish(116, 37, ">");
    for (std::size_t i = start; i < end; ++i) {
        const float x = 16 + static_cast<float>(i - start) * step;
        HAL::drawEnglish(x, 37, std::string(1, password[i]));
        if (input.editSelectedCharacter() && i == pos) highlight(x - 2, 26, step, 13);
    }

    HAL::drawChinese(24, 52, "保存");
    HAL::drawChinese(80, 52, "取消");
    if (input.editSelectedSave()) highlight(19, 42, 37, 12);
    else if (input.editSelectedCancel()) highlight(75, 42, 37, 12);
    HAL::drawChinese(8, 62, fitText(input.error().empty() ? "长按删除 返回撤销" : input.error(), 112));
}

// For password preview, display its tail plus a truncation marker and the
// real character count. Full plaintext remains browsable in edit mode.
std::string passwordPreview(const WifiPasswordState &input) {
    auto visible = input.previewPassword();
    const std::string length = " (" + std::to_string(input.password().size()) + ")";
    const float available = std::max(8.0f, 113.0f - HAL::getFontWidth(length));
    bool clipped = false;
    while (!visible.empty() && HAL::getFontWidth(visible) > available - (clipped ? HAL::getFontWidth("...") : 0)) {
        visible.erase(visible.begin());
        clipped = true;
    }
    if (clipped) visible = "..." + visible;
    return visible + length;
}

void drawWifiKeyboard(const std::string &ssid, const WifiPasswordState &input,
                      bool legacyWep) {
    if (input.stage() == WifiPasswordState::Stage::EditPassword) {
        drawEditPassword(input);
        return;
    }
    HAL::setDrawType(1);
    HAL::drawRFrame(2, 1, 124, 62, 3);
    HAL::setFont(astra::getUIConfig().mainFont);
    HAL::drawChinese(7, 11, legacyWep ? "WiFi密码 WEP" : "WiFi密码 8-63位");
    HAL::drawChinese(7, 22, fitText(ssid, 112));
    HAL::drawEnglish(7, 32, passwordPreview(input));
    HAL::drawHLine(6, 35, 116);

    if (input.stage() == WifiPasswordState::Stage::Categories) {
        const std::string row = std::string("分类: ") + input.categoryLabel()
            + " " + std::to_string(input.categoryPosition() + 1) + "/8";
        HAL::drawChinese(8, 46, fitText(row, 111));
        HAL::drawChinese(8, 60, fitText(
            input.error().empty() ? "短按切换 长按选择" : input.error(), 111));
    } else if (input.stage() == WifiPasswordState::Stage::Characters) {
        drawCharacterCarousel(input);
    }
}

void drawConfirm(const std::string &ssid, const std::string &action, int selected) {
    HAL::setDrawType(1);
    HAL::drawRFrame(4, 3, 120, 54, 3);
    HAL::setFont(astra::getUIConfig().mainFont);
    HAL::drawChinese(11, 16, "确认" + action + "WiFi?");
    HAL::drawChinese(11, 28, fitText(ssid, 108));
    HAL::drawHLine(9, 31, 110);
    HAL::drawChinese(19, 45, "执行");
    HAL::drawChinese(78, 45, "取消");
    HAL::setDrawType(2);
    HAL::drawRBox(selected == 0 ? 10 : 68, 34, 50, 13, 2);
    HAL::setDrawType(1);
}

} // namespace

bool inputWifiPassword(const std::string &ssid, bool legacyWep, std::string &password) {
    WifiPasswordState input(legacyWep ? 5 : 8, legacyWep ? 13 : 63, legacyWep);
    do {
        astra::Launcher::update(true, [&](astra::Clocker &, key::keyIndex key) {
            switch (key) {
                case key::KEY_PREV: input.press(WifiPasswordState::Key::Previous); break;
                case key::KEY_NEXT: input.press(WifiPasswordState::Key::Next); break;
                case key::KEY_CONFIRM: input.press(WifiPasswordState::Key::Confirm); break;
                case key::KEY_CANCEL: input.press(WifiPasswordState::Key::Cancel); break;
                default: break;
            }
            if (!input.done()) drawWifiKeyboard(ssid, input, legacyWep);
        });
    } while (!input.done());
    if (input.submitted()) password = input.password();
    input.clearPassword();
    return !password.empty() && input.submitted();
}

bool confirmWifiOperation(const std::string &ssid, const std::string &action) {
    NetworkConfirmationState confirmation;
    do {
        astra::Launcher::update(true, [&](astra::Clocker &, key::keyIndex key) {
            switch (key) {
                case key::KEY_PREV: confirmation.press(NetworkConfirmationState::Key::Previous); break;
                case key::KEY_NEXT: confirmation.press(NetworkConfirmationState::Key::Next); break;
                case key::KEY_CONFIRM: confirmation.press(NetworkConfirmationState::Key::Confirm); break;
                case key::KEY_CANCEL: confirmation.press(NetworkConfirmationState::Key::Cancel); break;
                default: break;
            }
            if (!confirmation.finished()) drawConfirm(ssid, action, confirmation.selected());
        });
    } while (!confirmation.finished());
    return confirmation.confirmed();
}
