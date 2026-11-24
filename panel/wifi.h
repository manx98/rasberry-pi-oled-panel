//
// Created by wenyiyu on 2024/10/28.
//

#ifndef RASPBERRY_PI_OLED_PANEL_WIFI_H
#define RASPBERRY_PI_OLED_PANEL_WIFI_H
#include "astra/ui/launcher.h"
#include "utils/wifi.h"

class WifiScanListEvent : public astra::List::Event {
private:
    std::list<RPI::ConnectionInfo> m_connections;
    std::list<RPI::WifiInfo> m_networks;
    bool refresh_requested_ = false;
    std::string device_name_;
    void populate(astra::Menu *current);
public:
    ~WifiScanListEvent() override = default;

    explicit WifiScanListEvent(std::string device_name);
    void requestRefresh() { refresh_requested_ = true; }

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
    const std::list<RPI::ConnectionInfo> *m_connections;
    WifiScanListEvent *m_owner;
public:
    WifiOperationEvent(const std::list<RPI::ConnectionInfo> *connections, RPI::WifiInfo wifi,
                       std::string device_name, WifiScanListEvent *owner);

    ~WifiOperationEvent() override = default;

    bool beforeOpen(astra::Menu *current) override;

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};

class WifiOperationActionEvent : public astra::List::Event {
public:
    WifiOperationActionEvent(RPI::WifiInfo info, std::string device,
                             std::string savedConnectionUuid, bool disconnect,
                             WifiScanListEvent *owner);
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override {
        return true;
    }
private:
    RPI::WifiInfo info_;
    std::string device_;
    std::string savedConnectionId_;
    bool disconnect_;
    WifiScanListEvent *owner_;
};

// Saved profiles are browsable independently of access-point scanning.
// One SSID can have multiple profiles, so actions always use the exact UUID.
class WifiSavedProfileEvent : public astra::List::Event {
public:
    WifiSavedProfileEvent(RPI::ConnectionInfo saved, std::string device,
                          WifiScanListEvent *owner);
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *, const std::vector<float> &,
                      astra::Clocker &) override { return true; }
private:
    RPI::ConnectionInfo saved_;
    std::string device_;
    WifiScanListEvent *owner_;
};

class WifiSavedActionEvent : public astra::List::Event {
public:
    WifiSavedActionEvent(RPI::ConnectionInfo saved, std::string device,
                         WifiScanListEvent *owner, bool forget);
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *, const std::vector<float> &,
                      astra::Clocker &) override { return true; }
private:
    RPI::ConnectionInfo saved_;
    std::string device_;
    WifiScanListEvent *owner_;
    bool forget_;
};

class WifiOperationForgetEvent: public astra::List::Event {
private:
    RPI::ConnectionInfo m_info;
    WifiScanListEvent *owner_;
public:
    WifiOperationForgetEvent(RPI::ConnectionInfo info, WifiScanListEvent *owner);
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override;
};
#endif //RASPBERRY_PI_OLED_PANEL_WIFI_H
