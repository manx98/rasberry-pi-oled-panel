#include "panel/status_idle_settings.h"
#include "panel/status_idle_config.h"
#include "astra/ui/launcher.h"
#include "astra/hal/hal.h"

#include <algorithm>
#include <string>

namespace status_idle {

std::string menuLabel() {
    const int seconds = Config::instance().seconds();
    return seconds == 0 ? "状态隐藏: 关闭" : "状态隐藏: " + std::to_string(seconds) + "秒";
}

bool SettingsEvent::beforeOpen(astra::Menu *current) {
    int proposed = Config::instance().seconds();
    bool finished = false;
    bool accepted = false;
    const auto *font = astra::getUIConfig().mainFont;
    while (!finished) {
        astra::Launcher::update(true, [&](astra::Clocker &, key::keyIndex key) {
            switch (key) {
                case key::KEY_PREV: proposed = std::min(300, proposed + 5); break;
                case key::KEY_NEXT: proposed = std::max(0, proposed - 5); break;
                case key::KEY_CONFIRM: accepted = true; finished = true; break;
                case key::KEY_CANCEL: finished = true; break;
                default: break;
            }
            HAL::setDrawType(1);
            HAL::drawRFrame(3, 2, 122, 52, 3);
            HAL::setFont(font);
            HAL::drawChinese(13, 16, "状态页自动隐藏");
            HAL::drawChinese(38, 35, proposed == 0 ? "关闭" : std::to_string(proposed) + "秒");
            HAL::drawChinese(13, 48, "上:+5 下:-5");
        });
    }
    if (accepted) {
        if (!Config::instance().setSeconds(proposed)) {
            astra::Launcher::popInfo({0, {{"隐藏时间保存失败", font}}}, 1300);
        } else {
            current->title.setText(0, {menuLabel(), font});
        }
    }
    return false; // This is an action, not a nested page.
}

} // namespace status_idle
