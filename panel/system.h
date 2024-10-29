//
// Created by wenyiyu on 2024/10/28.
//

#ifndef RASPBERRY_PI_OLED_PANEL_PANEL_SYSTEM_H
#define RASPBERRY_PI_OLED_PANEL_PANEL_SYSTEM_H

#include "astra/ui/launcher.h"
#include "utils/utils.h"

class SystemStatusListEvent : public astra::List::Event {
private:
    std::string net_card_name;
    RPI::NetworkStats stats;
    std::int64_t last_update_time{};
    std::int64_t last_render_time{};
public:
    ~SystemStatusListEvent() override = default;

    astra::Text getCpuInfo();

    astra::TextBox getIpInfo();

    astra::Text getDiskInfo();

    astra::Text getMemInfo();

    astra::Text getSpeedInfo();

    void refresh(astra::Menu *current);

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class InputNumberListEvent : public astra::List::Event {
public:
    ~InputNumberListEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

#endif //RASPBERRY_PI_OLED_PANEL_PANEL_SYSTEM_H
