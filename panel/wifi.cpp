//
// Created by wenyiyu on 2024/10/28.
//

#include <thread>
#include <atomic>
#include <utility>
#include <spdlog/fmt/fmt.h>
#include "text.h"
#include "wifi.h"
#include "utils/wifi.h"
#include "spdlog/spdlog.h"

static bool sortWifiInfoStrengthCb(const RPI::WifiInfo &a, const RPI::WifiInfo &b) {
    return a.strength > b.strength;
}

WifiScanListEvent::WifiScanListEvent(const std::list<RPI::ConnectionInfo> *connections, std::string _device_name)
        : m_connections(connections), device_name_(std::move(_device_name)) {}

bool WifiScanListEvent::beforeOpen(astra::Menu *current) {
    astra::TextBox title(0, {{SCANNING_TEXT, astra::getUIConfig().mainFont}});
    std::atomic<bool> finished{false};
    std::thread scan_thread([&]() {
        auto infos = RPI::getWifiList(device_name_);
        infos.sort(sortWifiInfoStrengthCb);
        current->clear();
        auto font = astra::getUIConfig().mainFont;
        for (const auto &item: infos) {
            current->addItem(new astra::List(
                    {0,
                     {{fmt::format("[{:02d}] {}", item.strength, item.SSID.empty() ? EMPTY_SSID : item.SSID), font}}},
                    new WifiOperationEvent(m_connections, item, device_name_)));
        }
        finished = true;
    });
    auto screenWeight = HAL::getSystemConfig().screenWeight;
    astra::Launcher::progress(screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                  return !finished;
                              });
    scan_thread.join();
    return true;
}

bool WifiScanListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

bool WifiDeviceEvent::beforeOpen(astra::Menu *current) {
    std::atomic<bool> finished{false};
    std::thread t([&]() {
        m_connections = RPI::get_connections();
        auto devices = RPI::getWifiDevices();
        current->clear();
        auto font = astra::getUIConfig().mainFont;
        for (const auto &item: devices) {
            astra::TextBox title = {1, {{"- " + item.iface, font}}};
            if (!item.SSID.empty()) {
                title.add({"  " + item.SSID, font});
            }
            current->addItem(new astra::List(title, new WifiScanListEvent(&m_connections, item.iface)));
        }
        finished = true;
    });
    astra::TextBox title(0, {{LOADING_DEVICE_TEXT, astra::getUIConfig().mainFont}});
    astra::Launcher::progress(HAL::getSystemConfig().screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                  return !finished;
                              });
    t.join();
    return true;
}

bool WifiDeviceEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

bool WifiOperationEvent::beforeOpen(astra::Menu *current) {
    current->clear();
    current->addItem(new astra::List({0, {
            {m_info.SSID.empty() ? EMPTY_SSID : m_info.SSID, astra::getUIConfig().mainFont},
            {m_info.BSS, astra::getUIConfig().mainFont},
            {fmt::format("[{:02d}] {} MHz", nm_utils_wifi_freq_to_channel(m_info.frequency),
                         m_info.frequency), astra::getUIConfig().mainFont}
    }}));
    std::unique_ptr<RPI::ConnectionInfo> info(RPI::get_active_connection(m_device_name));
    bool active = false;
    if (info) {
        spdlog::info("active connection, id={}, ssid={}, bssid={}", info->id, info->ssid, info->bssid);
        if (info->bssid.empty()) {
            active = m_info.SSID == info->ssid;
        } else {
            active = m_info.BSS == info->bssid;
        }
    }
    current->addItem(
            new astra::List({0, {{active ? DISCONNECT_WIFI_TEXT : CONNECT_WIFI_TEXT, astra::getUIConfig().mainFont}}}));
    if(m_connections) {
        for (const auto &item: *m_connections)
        {
            bool is_config;
            if(item.bssid.empty()) {
                is_config = item.ssid == m_info.SSID;
            } else {
                is_config = item.ssid == m_info.BSS;
            }
            if(is_config) {
                current->addItem(new astra::List({0, {{FORGET_WIFI_TEXT, astra::getUIConfig().mainFont}}}, new WifiOperationForgetEvent(item)));
                break;
            }
        }
    }
    return true;
}

bool
WifiOperationEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

WifiOperationEvent::WifiOperationEvent(const std::list<RPI::ConnectionInfo> *connections, RPI::WifiInfo _wifi,
                                       std::string device_name) :
        m_connections(connections), m_info(std::move(_wifi)), m_device_name(std::move(device_name)) {
}

WifiOperationForgetEvent::WifiOperationForgetEvent(RPI::ConnectionInfo  info): m_info(std::move(info)) {
}

bool WifiOperationForgetEvent::beforeOpen(astra::Menu *current) {
    if(current->parent) {
        current->parent->removeItem(current);
    }
    return false;
}

bool WifiOperationForgetEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera,
                                            astra::Clocker &clocker) {
    return false;
}
