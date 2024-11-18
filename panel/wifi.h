//
// Created by wenyiyu on 2024/10/28.
//

#ifndef RASPBERRY_PI_OLED_PANEL_WIFI_H
#define RASPBERRY_PI_OLED_PANEL_WIFI_H
#include "astra/ui/launcher.h"
#include "utils/wifi.h"

class WifiScanListEvent : public astra::List::Event {
private:
    std::string device_name_;
public:
    ~WifiScanListEvent() override = default;

    explicit WifiScanListEvent(std::string _device_name);

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiDeviceEvent : public astra::List::Event {
public:
    ~WifiDeviceEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiOperationEvent : public astra::List::Event {
private:
    RPI::WifiInfo m_info;
    std::string m_device_name;
public:
    WifiOperationEvent(const RPI::WifiInfo& _wifi, std::string device_name);

    ~WifiOperationEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};
#endif //RASPBERRY_PI_OLED_PANEL_WIFI_H
