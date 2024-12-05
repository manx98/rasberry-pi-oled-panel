//
// Created by wenyiyu on 2024/10/28.
//

#ifndef RASPBERRY_PI_OLED_PANEL_WIFI_H
#define RASPBERRY_PI_OLED_PANEL_WIFI_H
#include "astra/ui/launcher.h"
#include "utils/wifi.h"

class WifiScanListEvent : public astra::List::Event {
private:
    const std::list<RPI::ConnectionInfo> *m_connections;
    std::string device_name_;
public:
    ~WifiScanListEvent() override = default;

    explicit WifiScanListEvent(const std::list<RPI::ConnectionInfo> *connections, std::string _device_name);

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiDeviceEvent : public astra::List::Event {
private:
    std::list<RPI::ConnectionInfo> m_connections;
public:
    ~WifiDeviceEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiOperationEvent : public astra::List::Event {
private:
    RPI::WifiInfo m_info;
    std::string m_device_name;
    const std::list<RPI::ConnectionInfo> *m_connections;
public:
    WifiOperationEvent(const std::list<RPI::ConnectionInfo> *connections, RPI::WifiInfo  _wifi, std::string device_name);

    ~WifiOperationEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiOperationForgetEvent: public astra::List::Event {
private:
    RPI::ConnectionInfo m_info;
public:
    explicit WifiOperationForgetEvent(RPI::ConnectionInfo  info);
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};
#endif //RASPBERRY_PI_OLED_PANEL_WIFI_H
