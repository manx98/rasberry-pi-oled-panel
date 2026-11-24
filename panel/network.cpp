//
// Created by wenyiyu on 2024/10/28.
//

#include <thread>
#include <utility>
#include <exception>
#include <cstdio>
#include "panel/network_confirmation.h"
#include "panel/network_list_layout.h"
#include <atomic>
#include "utils/utils.h"
#include "utils/shutdown.h"
#include "text.h"
#include "network.h"

void NetWorkDeviceListEvent::enable_device(const std::string &device, bool &enable) {
    const bool requested = enable;
    // Never modify an interface without a second, explicit user action.
    // Disabling an SSH-carrying interface can immediately sever remote access.
    if (!confirmNetworkInterfaceChange(device, requested)) {
        enable = !requested; // Keep the checkbox in its original state.
        return;
    }

    if (!RPI::setIpLinkUp(device.c_str(), requested)) {
        enable = !requested; // Only revert the visual state after failure.
        astra::Launcher::popInfo(
            {1, {{requested ? ENABLE_FAIL_TEXT : DISABLE_FAIL_TEXT, astra::getUIConfig().mainFont}}}, 800);
        return;
    }
    astra::Launcher::popInfo(
        {1, {{requested ? ENABLE_SUCCESS_TEXT : DISABLE_SUCCESS_TEXT, astra::getUIConfig().mainFont}}}, 800);
}

bool NetWorkDeviceListEvent::beforeOpen(astra::Menu *current) {
    astra::TextBox title(0, {{LOADING_DATA_TEXT, astra::getUIConfig().mainFont}});
    std::atomic<bool> finished{false};
    std::map<std::string, RPI::NetworkInfo> infos;
    std::thread scan_thread([&]() {
        try {
            infos = RPI::getNetworkInfos();
        } catch (const std::exception &e) {
            std::fprintf(stderr, "Read network interfaces failed: %s\n", e.what());
        }
        finished = true;
    });
    auto screenWeight = HAL::getSystemConfig().screenWeight;
    panel_shutdown::JoinThread join(scan_thread);
    astra::Launcher::progress(screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                  return !finished;
                              });
    scan_thread.join();
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    current->initFlag = false;
    current->listItemGap = 2.0f;
    // Only the focused interface is allowed to scroll an unusually long
    // address/name; unrelated rows must remain readable and stationary.
    current->scrollSelectedTextOnly = true;
    auto font = astra::getUIConfig().mainFont;
    const auto &ui = astra::getUIConfig();
    // Leave enough horizontal room for the enable/disable checkbox and the
    // right-side scrollbar. Recalculate after attaching the widget as Astra
    // initially measures a List before it owns the checkbox.
    const float textWidth = screenWeight - ui.listBarWeight - ui.checkBoxRightMargin
                            - ui.checkBoxWidth - 2.0f * ui.listTextMargin;
    for (const auto &item : infos) {
        const auto row = network_list::compactRow(item.first, item.second.inet,
                                                  item.second.inet6);
        astra::TextBox text{1, {{row.name, font}}};
        // Do not create a three-line list item with full IPv4 + IPv6. Three
        // interfaces including a down interface now fit within 64 pixels.
        if (!row.address.empty()) text.add({row.address, font});
        auto *entry = new astra::List(text);
        current->addItem(entry,
                         new astra::CheckBox(item.second.up, [this, name = item.first](bool &checked) {
                             enable_device(name, checked);
                         }));
        entry->title.setMaxWidth(textWidth);
    }
    if (infos.empty()) current->addItem(new astra::List({0, {{"没有网络接口", font}}}));
    return true;
}

bool
NetWorkDeviceListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}
