//
// Created by wenyiyu on 2024/10/28.
//

#include <thread>
#include <utility>
#include <exception>
#include <cstdio>
#include "panel/network_confirmation.h"
#include <atomic>
#include "utils/utils.h"
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
    astra::Launcher::progress(screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                  return !finished;
                              });
    scan_thread.join();
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    auto font = astra::getUIConfig().mainFont;
    for (const auto &item : infos) {
        astra::TextBox text{1, {{"- " + item.first, font}}};
        if (!item.second.inet6.empty()) text.add({"  " + item.second.inet6, font});
        if (!item.second.inet.empty()) text.add({"  " + item.second.inet, font});
        current->addItem(new astra::List(text),
                         new astra::CheckBox(item.second.up, [this, name = item.first](bool &checked) {
                             enable_device(name, checked);
                         }));
    }
    if (infos.empty()) current->addItem(new astra::List({0, {{"没有网络接口", font}}}));
    return true;
}

bool
NetWorkDeviceListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}