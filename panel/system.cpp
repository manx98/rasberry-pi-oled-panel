//
// Created by wenyiyu on 2024/10/28.
//

#include "system.h"
#include "panel/status_idle_config.h"
#include "text.h"
#include "spdlog/spdlog.h"
#include <spdlog/fmt/fmt.h>

#define STATUS_FONT u8g2_font_boutique_bitmap_9x9_t_gb2312b
std::string defaultNetworkName[]{"wlan0", "eth0"};
#ifndef STATUS_UPDATE_INTERVAL_MILL
#define STATUS_UPDATE_INTERVAL_MILL 1000
#endif

astra::Text SystemStatusListEvent::getCpuInfo() {
    auto rate = RPI::getCpuRate();
    auto temp = RPI::getThermalZoneTemp();
    return {fmt::format("CPU {:.2f}% {:.2f}℃", rate, temp), STATUS_FONT};
}

astra::TextBox SystemStatusListEvent::getIpInfo() {
    auto infos = RPI::getNetworkInfos();
    std::string ip;
    std::string name;
    for (const auto &default_name: defaultNetworkName) {
        auto it = infos.find(default_name);
        if (it != infos.end()) {
            name = default_name;
            auto info = it->second;
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
    }
    if (name.empty()) {
        for (const auto &item: infos) {
            auto &info = item.second;
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
    }
    if (name.empty()) {
        net_card_name.clear();
        last_update_time = RPI::getUnixMill();
        stats = {};
    } else if (net_card_name != name) {
        net_card_name = name;
        last_update_time = RPI::getUnixMill();
        stats = RPI::getNetworkStats(net_card_name);
    }
    // Only the IP is displayed; retain the interface name internally for
    // network rate counters. Keep a single line in the compact status list.
    return {0.0f, {{ip.empty() ? "无IP" : ip, STATUS_FONT}}};
}

astra::Text SystemStatusListEvent::getDiskInfo() {
    auto info = RPI::getDiskInfo("/");
    return {fmt::format("Disk {}/{}", RPI::formatSize(info.total - info.free), RPI::formatSize(info.total)),
            STATUS_FONT};
}

astra::Text SystemStatusListEvent::getMemInfo() {
    auto info = RPI::getMemoryInfo();
    return {fmt::format("Mem {}/{}", RPI::formatSize(info.total - info.free - info.buffers - info.cached),
                        RPI::formatSize(info.total)),
            STATUS_FONT};
}

astra::Text SystemStatusListEvent::getSpeedInfo() {
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

void SystemStatusListEvent::refresh(astra::Menu *current) {
    // Only the system-status page is compact; Wi-Fi and other menus use
    // Astra's standard list spacing. The IP/CPU/Mem/Disk/Speed status
    // information now occupies five lines within the 64px screen.
    // Keep a 2px inter-row pitch so the selected background has 1px of
    // breathing room under the text, while remaining much tighter than 4px.
    current->listItemGap = 2.0f;
    current->listSelectorTopInset = 1.0f;
    current->hideNavigationAfterIdle = true;
    current->navigationIdleTimeoutMs = static_cast<std::uint64_t>(
        status_idle::Config::instance().seconds()) * 1000;
    current->navigationHidden = false;
    current->clear();
    current->selectIndex = 0;
    auto textMargin = astra::getUIConfig().listTextMargin;
    current->addItem(new astra::List(getIpInfo()));
    current->addItem(new astra::List({textMargin, {getCpuInfo()}}));
    current->addItem(new astra::List({textMargin, {getMemInfo()}}));
    current->addItem(new astra::List({textMargin, {getDiskInfo()}}));
    current->addItem(new astra::List({textMargin, {getSpeedInfo()}}));
}

bool SystemStatusListEvent::beforeOpen(astra::Menu *current) {
    last_render_time = RPI::getUnixMill();
    refresh(current);
    return true;
}

bool
SystemStatusListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    auto now = RPI::getUnixMill();
    if (now - last_render_time > STATUS_UPDATE_INTERVAL_MILL) {
        last_render_time = now;
        current->childMenu[0]->
                title = getIpInfo();
        current->childMenu[1]->title.setText(0,

                                             getCpuInfo()

        );
        current->childMenu[2]->title.setText(0,

                                             getMemInfo()

        );
        current->childMenu[3]->title.setText(0,

                                             getDiskInfo()

        );
        current->childMenu[4]->title.setText(0,

                                             getSpeedInfo()

        );
    }
    return true;
}


bool InputNumberListEvent::beforeOpen(astra::Menu *current) {
    auto result = astra::Launcher::numbersChoose(STATUS_FONT);
    spdlog::info("InputNumberListEvent result: {}", result);
    return false;
}

bool
InputNumberListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return false;
}