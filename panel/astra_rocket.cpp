//
// Created by Fir on 2024/3/7 007.
//

//todo warning flash used age >= 90.93%, will be damaged soon

#include <utility>
#include <vector>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <u8g2.h>
#include <thread>
#include <atomic>
#include "astra_rocket.h"
#include "astra_logo.h"
#include "sh1106_hal/sh1106_hal.h"
#include "icons/app_icons.h"
#include "utils/utils.h"

#ifndef STATUS_UPDATE_INTERVAL_MILL
#define STATUS_UPDATE_INTERVAL_MILL 1000
#endif

#define STATUS_FONT u8g2_font_boutique_bitmap_9x9_t_gb2312b

bool test = false;
unsigned char testIndex = 0;
unsigned char testSlider = 60;

class WifiScanListEvent : public astra::List::Event {
    std::string device_name_;
public:
    ~WifiScanListEvent() override = default;

    explicit WifiScanListEvent(std::string _device_name) : device_name_(std::move(_device_name)) {}

    bool beforeOpen(astra::Menu *current) override {
        astra::TextBox title(0, {{"正在扫描...", astra::getUIConfig().mainFont}});
        std::atomic<bool> finished{false};
        std::thread scan_thread([&]() {
            auto infos = RPI::getWifiList(device_name_);
            current->clear();
            auto font = astra::getUIConfig().mainFont;
            for (const auto &item: infos) {
                current->addItem(new astra::List({1, {{item.BSS, font},
                                                      {"SSID:" + item.SSID, font},
                                                      {fmt::format("SIGNAL:{}dBm", item.signal), font}}}));
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

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override {
        return true;
    }
};

class WifiDeviceEvent : public astra::List::Event {
public:
    ~WifiDeviceEvent() override = default;

    bool beforeOpen(astra::Menu *current) override {
        std::atomic<bool> finished{false};
        std::thread t([&]() {
            auto devices = RPI::getWifiDevices();
            current->clear();
            auto font = astra::getUIConfig().mainFont;
            for (const auto &item: devices) {
                auto info = RPI::getWifiConnectInfo(item);
                current->addItem(new astra::List({1, {{"-Dev: " + item, font}, {" SSID: " + info->SSID, font}}},
                                                 new WifiScanListEvent(item)));
            }
            finished = true;
        });
        astra::TextBox title(0, {{"正在加载设备列表...", astra::getUIConfig().mainFont}});
        auto screenWeight = HAL::getSystemConfig().screenWeight;
        astra::Launcher::progress(screenWeight - 20, &title, nullptr,
                                  [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                      return !finished;
                                  });
        t.join();
        return true;
    }

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override {
        return true;
    }
};

class SystemStatusListEvent : public astra::List::Event {
private:
    std::string net_card_name;
    RPI::NetworkStats stats;
    std::int64_t last_update_time;
    std::int64_t last_render_time;
public:
    ~SystemStatusListEvent() override = default;

    astra::Text getCpuInfo() {
        auto rate = RPI::getCpuRate();
        auto temp = RPI::getThermalZoneTemp();
        return {fmt::format("CPU {:.2f}% {:.2f}℃", rate, temp), STATUS_FONT};
    }

    astra::TextBox getIpInfo() {
        auto infos = RPI::getNetworkInfos();
        std::string ip;
        std::string name;
        for (const auto &item: infos) {
            auto &info = item.second;
            if (info.name == "lo") {
                continue;
            }
            if (!info.inet.empty()) {
                name = info.name;
                ip = info.inet;
                break;
            } else if (!info.inet6.empty()) {
                name = info.name;
                ip = info.inet6;
                break;
            }
        }
        if (name.empty()) {
            last_update_time = RPI::getUnixMill();
            stats = {};
        } else if (net_card_name != name) {
            net_card_name = name;
            last_update_time = RPI::getUnixMill();
            stats = RPI::getNetworkStats(net_card_name);
        }
        return {astra::getUIConfig().listTextMargin, {{name, STATUS_FONT}, {ip, STATUS_FONT}}};
    }

    astra::Text getDiskInfo() {
        auto info = RPI::getDiskInfo("/");
        return {fmt::format("Disk {}/{}", RPI::formatSize(info.total - info.free), RPI::formatSize(info.total)), STATUS_FONT};
    }

    astra::Text getMemInfo() {
        auto info = RPI::getMemoryInfo();
        return {fmt::format("Mem {}/{}", RPI::formatSize(info.total - info.free), RPI::formatSize(info.total)), STATUS_FONT};
    }

    astra::Text getSpeedInfo() {
        float tx_speed{};
        float rx_speed{};
        if (!net_card_name.empty()) {
            auto now = RPI::getUnixMill();
            auto now_stats = RPI::getNetworkStats(net_card_name);
            auto rx = now_stats.rxBytes - stats.rxBytes;
            auto tx = now_stats.txBytes - stats.txBytes;
            stats = now_stats;
            auto used = now - last_update_time;
            last_update_time = now;
            if (used > 0) {
                if (rx > 0) {
                    rx_speed = static_cast<float>(rx) / static_cast<float>(used) * 1000;
                }
                if (tx > 0) {
                    tx_speed = static_cast<float>(tx) / static_cast<float>(used) * 1000;
                }
            }
        }
        return {fmt::format("↑{}/s ↓{}/s", RPI::formatSize(tx_speed), RPI::formatSize(rx_speed)), STATUS_FONT};
    }

    void refresh(astra::Menu *current) {
        auto textMarin = astra::getUIConfig().listTextMargin;
        current->addItem(new astra::List(getIpInfo()));
        current->addItem(new astra::List({textMarin, {getCpuInfo()}}));
        current->addItem(new astra::List({textMarin, {getMemInfo()}}));
        current->addItem(new astra::List({textMarin, {getDiskInfo()}}));
        current->addItem(new astra::List({textMarin, {getSpeedInfo()}}));
    }

    bool beforeOpen(astra::Menu *current) override {
        last_render_time = RPI::getUnixMill();
        current->clear();
        auto textMarin = astra::getUIConfig().listTextMargin;
        current->addItem(new astra::List(getIpInfo()));
        current->addItem(new astra::List({textMarin, {getCpuInfo()}}));
        current->addItem(new astra::List({textMarin, {getMemInfo()}}));
        current->addItem(new astra::List({textMarin, {getDiskInfo()}}));
        current->addItem(new astra::List({textMarin, {getSpeedInfo()}}));
        return true;
    }

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override {
        auto now = RPI::getUnixMill();
        if (now - last_render_time > STATUS_UPDATE_INTERVAL_MILL) {
            last_render_time = now;
            current->childMenu[0]->title = getIpInfo();
            current->childMenu[1]->title.setText(0, getCpuInfo());
            current->childMenu[2]->title.setText(0, getMemInfo());
            current->childMenu[3]->title.setText(0, getDiskInfo());
            current->childMenu[4]->title.setText(0, getSpeedInfo());
        }
        return true;
    }
};

class InputNumberListEvent : public astra::List::Event {
public:
    ~InputNumberListEvent() override = default;

    bool beforeOpen(astra::Menu *current) override {
        auto result = astra::Launcher::numbersChoose(STATUS_FONT);
        spdlog::info("InputNumberListEvent result: {}", result);
        return false;
    }

    bool beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) override {
        return false;
    }
};
void astraCoreInit(void) {
    HAL::inject(new Sh1106Hal());

    HAL::delay(350);
//  astra::drawLogo(5000);
    auto font = astra::getUIConfig().mainFont;
    auto textMarin = astra::getUIConfig().listTextMargin;
    auto *rootPage = new astra::Tile({textMarin, {{"root", font}}});
    rootPage->addItem(
            new astra::List({textMarin, {{"系统状态", font}}}, system_status_icon_30x30, new SystemStatusListEvent()));
    rootPage->addItem(new astra::List({textMarin, {{"WIFI", font}}}, wifi_icon_30x30, new WifiDeviceEvent()));
    rootPage->addItem(new astra::List({textMarin, {{"网络接口", font}}}, network_interface_icon_30x30, new InputNumberListEvent()));
    auto *secondPage = new astra::List({textMarin, {{"设置", font}}}, setting_icon_30x30);
    rootPage->addItem(secondPage);

    secondPage->addItem(new astra::List({textMarin, {{"-测试2", font}}}), new astra::CheckBox(test, [](bool checked) {
        printf("checked --------> %d\n", checked);
    }));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试4", font}}}),
                        new astra::Slider("测试", 0, 100, 50, testSlider));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试5", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {
            {"-111111", font},
            {" 222222", font},
    }}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));

    astra::Launcher::init(rootPage);
}

[[noreturn]] void astraCoreStart(void) {
    for (;;) {  //NOLINT
        astra::Launcher::update();
    }
}

void astraCoreDestroy(void) {
    HAL::destroy();
}
