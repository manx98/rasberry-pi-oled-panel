#include "panel/network_confirmation.h"
#include "panel/network_confirmation_state.h"
#include "astra/ui/launcher.h"
#include "astra/hal/hal.h"

namespace {
void drawUnderlyingMenu(astra::Clocker &clocker) {
    auto *menu = astra::Launcher::getCurrentMenu();
    auto *camera = astra::Launcher::getCamera();
    auto *selector = astra::Launcher::getSelector();
    menu->doRender(camera->getPosition(), clocker);
    selector->render(camera->getPosition(), clocker);
    camera->update(menu, selector, clocker);
}

void drawConfirmDialog(const std::string &interfaceName, bool enable, int selection) {
    HAL::setDrawType(0);
    HAL::drawRBox(4, 3, 120, 49, 3);
    HAL::setDrawType(1);
    HAL::drawRFrame(4, 3, 120, 49, 3);
    HAL::setFont(astra::getUIConfig().mainFont);

    HAL::drawChinese(12, 16, enable ? "确认启用网卡?" : "确认禁用网卡?");
    // Keep long interface names within the 128x64 OLED popup.
    std::string label = interfaceName;
    while (label.size() > 1 && HAL::getFontWidth(label) > 101) label.pop_back();
    HAL::drawChinese(12, 29, label);
    HAL::drawHLine(10, 31, 108);
    HAL::drawChinese(20, 45, "执行");
    HAL::drawChinese(75, 45, "取消");
    HAL::setDrawType(2); // XOR selection box leaves text legible.
    HAL::drawRBox(selection == 0 ? 11 : 67, 34, 51, 13, 2);
    HAL::setDrawType(1);
}
} // namespace

bool confirmNetworkInterfaceChange(const std::string &interfaceName, bool enable) {
    NetworkConfirmationState state;
    do {
        astra::Launcher::update(true, [&](astra::Clocker &clocker, key::keyIndex key) {
            drawUnderlyingMenu(clocker);
            switch (key) {
                case key::KEY_PREV: state.press(NetworkConfirmationState::Key::Previous); break;
                case key::KEY_NEXT: state.press(NetworkConfirmationState::Key::Next); break;
                case key::KEY_CONFIRM: state.press(NetworkConfirmationState::Key::Confirm); break;
                case key::KEY_CANCEL: state.press(NetworkConfirmationState::Key::Cancel); break;
                default: break;
            }
            if (!state.finished()) drawConfirmDialog(interfaceName, enable, state.selected());
        });
    } while (!state.finished());
    return state.confirmed();
}
