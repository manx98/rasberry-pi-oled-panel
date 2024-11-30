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

static bool sortWifiInfoStrengthCb(const RPI::WifiInfo &a, const RPI::WifiInfo &b) {
    return a.strength > b.strength;
}

WifiScanListEvent::WifiScanListEvent(std::string _device_name) : device_name_(std::move(_device_name)) {}

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
                    {0, {{fmt::format("[{:02d}] {}", item.strength, item.SSID.empty() ? "--" : item.SSID), font}}},
                    new WifiOperationEvent(item, device_name_)));
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
        auto devices = RPI::getWifiDevices();
        current->clear();
        auto font = astra::getUIConfig().mainFont;
        for (const auto &item: devices) {
            astra::TextBox title = {1, {{"- " + item.iface, font}}};
            if (!item.SSID.empty()) {
                title.add({"  " + item.SSID, font});
            }
            current->addItem(new astra::List(title, new WifiScanListEvent(item.iface)));
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
    current->addItem(new astra::List({0, {{m_info.SSID, astra::getUIConfig().mainFont}}}));
    current->addItem(new astra::List({0, {{fmt::format("<BSS> {}", m_info.BSS), u8g2_font_tiny5_te}}}));
    current->addItem(new astra::List({0, {{fmt::format("<{}> {} MHz", nm_utils_wifi_freq_to_channel(m_info.frequency),
                                                       m_info.frequency), u8g2_font_tiny5_te}}}));
    return true;
}

bool
WifiOperationEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

WifiOperationEvent::WifiOperationEvent(const RPI::WifiInfo& _wifi, std::string device_name) : m_info(std::move(_wifi)), m_device_name(std::move(device_name)) {
}
