//
// Created by wenyiyu on 2024/10/28.
//

#ifndef RASPBERRY_PI_OLED_PANEL_PANEL_NETWORK_H
#define RASPBERRY_PI_OLED_PANEL_PANEL_NETWORK_H
#include <astra/ui/launcher.h>

class NetWorkDeviceListEvent : public astra::List::Event {
public:
    ~NetWorkDeviceListEvent() override = default;

    void enable_device(const std::string &device, bool &enable);

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker);
};


#endif //RASPBERRY_PI_OLED_PANEL_PANEL_NETWORK_H
