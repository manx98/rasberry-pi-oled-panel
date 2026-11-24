#include "panel/fan/fan_settings.h"
#include "panel/fan/fan_controller.h"
#include "astra/ui/launcher.h"
#include "astra/hal/hal.h"
#include <algorithm>
#include <chrono>
#include <cstdio>

namespace fan {
namespace {
using Event = astra::List::Event;

auto mainFont() { return astra::getUIConfig().mainFont; }
astra::TextBox label(const std::string &text) {
    return {astra::getUIConfig().listTextMargin, {{text, mainFont()}}};
}

std::string modeLabel(Mode mode) {
    switch (mode) {
        case Mode::Auto: return "模式: 自动";
        case Mode::AlwaysOn: return "模式: 常开";
        case Mode::Off: return "模式: 关闭";
    }
    return "模式: 未知";
}

std::string tempLabel(bool start, int degrees) {
    return std::string(start ? "启动温度: " : "停止温度: ") + std::to_string(degrees) + "℃";
}

std::string polarityLabel(bool high) { return high ? "有效电平: 高" : "有效电平: 低"; }

std::string statusLabel() {
    auto status = Controller::instance().status();
    if (!status.gpioOkay) return "风扇 GPIO 故障";
    const auto fanState = status.fanOn ? "开" : "关";
    if (!status.temperatureValid) return std::string("温度无效 风扇") + fanState;
    char temperature[24]{};
    std::snprintf(temperature, sizeof(temperature), "%.1f", status.temperatureC);
    return std::string(temperature) + "℃ 风扇" + fanState;
}

void showSaveError() {
    astra::Launcher::popInfo({0, {{"风扇设置保存失败", mainFont()}}}, 1300);
}

// 2-button numeric editor. Short taps adjust +/-1 °C; long GPIO24 saves,
// long GPIO23 cancels. No configuration is changed before confirmation.
bool editTemperature(bool start, int &value, const Config &base) {
    const int minimum = start ? std::max(35, base.offTemp + 2) : 30;
    const int maximum = start ? 85 : std::min(80, base.onTemp - 2);
    bool save = false;
    bool finished = false;
    while (!finished) {
        astra::Launcher::update(true, [&](astra::Clocker &, key::keyIndex key) {
            switch (key) {
                case key::KEY_PREV: value = std::min(maximum, value + 1); break;
                case key::KEY_NEXT: value = std::max(minimum, value - 1); break;
                case key::KEY_CONFIRM: save = true; finished = true; break;
                case key::KEY_CANCEL: finished = true; break;
                default: break;
            }
            HAL::setDrawType(1);
            HAL::drawRFrame(3, 2, 122, 52, 3);
            HAL::setFont(mainFont());
            HAL::drawChinese(14, 16, start ? "启动温度" : "停止温度");
            HAL::drawChinese(44, 35, std::to_string(value) + "℃");
            HAL::drawChinese(13, 48, "24:+1  23:-1");
            // SH1106 HAL draws its long-press progress overlay at the bottom.
        });
    }
    return save;
}

class OptionEvent final : public Event {
public:
    enum class Kind { Mode, OnTemp, OffTemp, Polarity };
    explicit OptionEvent(Kind kind) : kind_(kind) {}

    bool beforeOpen(astra::Menu *current) override {
        auto &ctl = Controller::instance();
        Config next = ctl.config();
        switch (kind_) {
            case Kind::Mode:
                next.mode = static_cast<Mode>((static_cast<int>(next.mode) + 1) % 3);
                break;
            case Kind::Polarity:
                next.activeHigh = !next.activeHigh;
                break;
            case Kind::OnTemp:
            case Kind::OffTemp: {
                int value = kind_ == Kind::OnTemp ? next.onTemp : next.offTemp;
                if (!editTemperature(kind_ == Kind::OnTemp, value, next)) return false;
                if (kind_ == Kind::OnTemp) next.onTemp = value;
                else next.offTemp = value;
                break;
            }
        }
        if (!ctl.update(next)) {
            showSaveError();
            return false;
        }
        const auto updated = ctl.config();
        if (kind_ == Kind::Mode) current->title.setText(0, {modeLabel(updated.mode), mainFont()});
        else if (kind_ == Kind::OnTemp) current->title.setText(0, {tempLabel(true, updated.onTemp), mainFont()});
        else if (kind_ == Kind::OffTemp) current->title.setText(0, {tempLabel(false, updated.offTemp), mainFont()});
        else current->title.setText(0, {polarityLabel(updated.activeHigh), mainFont()});
        return false; // an action, not a nested page
    }

    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override {
        return true;
    }
private:
    Kind kind_;
};

class StatusEvent final : public Event {
public:
    bool beforeOpen(astra::Menu *) override { return false; }
    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override { return true; }
};

long long monotonicMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
} // namespace

bool SettingsEvent::beforeOpen(astra::Menu *current) {
    auto c = Controller::instance().config();
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    current->addItem(new astra::List(label(modeLabel(c.mode)), new OptionEvent(OptionEvent::Kind::Mode)));
    current->addItem(new astra::List(label(tempLabel(true, c.onTemp)), new OptionEvent(OptionEvent::Kind::OnTemp)));
    current->addItem(new astra::List(label(tempLabel(false, c.offTemp)), new OptionEvent(OptionEvent::Kind::OffTemp)));
    current->addItem(new astra::List(label(polarityLabel(c.activeHigh)), new OptionEvent(OptionEvent::Kind::Polarity)));
    current->addItem(new astra::List(label(statusLabel()), new StatusEvent()));
    lastStatusRefreshMs_ = monotonicMs();
    return true;
}

bool SettingsEvent::beforeRender(astra::Menu *current, const std::vector<float> &, astra::Clocker &) {
    auto now = monotonicMs();
    if (now - lastStatusRefreshMs_ > 1000 && current->childMenu.size() >= 5) {
        current->childMenu[4]->title.setText(0, {statusLabel(), mainFont()});
        lastStatusRefreshMs_ = now;
    }
    return true;
}
} // namespace fan
