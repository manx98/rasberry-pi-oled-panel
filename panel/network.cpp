//
// Created by wenyiyu on 2024/10/28.
//

#include <thread>
#include <atomic>
#include "utils/utils.h"
#include "text.h"
#include "network.h"

void NetWorkDeviceListEvent::enable_device(const std::string &device, bool &enable) {
    if (!RPI::setIpLinkUp(device.c_str(), enable)) {
        enable = !enable;
        astra::Launcher::popInfo(
                {1, {{enable ? ENABLE_FAIL_TEXT : DISABLE_FAIL_TEXT, astra::getUIConfig().mainFont}}}, 600);
    } else {
        astra::Launcher::popInfo(
                {1, {{enable ? ENABLE_SUCCESS_TEXT : DISABLE_SUCCESS_TEXT, astra::getUIConfig().mainFont}}}, 600);
    }
}

bool NetWorkDeviceListEvent::beforeOpen(astra::Menu *current) {
    astra::TextBox title(0, {{LOADING_DATA_TEXT, astra::getUIConfig().mainFont}});
    std::atomic<bool> finished{false};
    std::thread scan_thread([&]() {
        auto infos = RPI::getNetworkInfos();
        current->clear();
        auto font = astra::getUIConfig().mainFont;
        for (const auto &item: infos) {
            astra::TextBox text{1, {{"- " + item.first, font}}};
            if (!item.second.inet6.empty()) {
                text.add({"  " + item.second.inet6, font});
            }
            if (!item.second.inet.empty()) {
                text.add({"  " + item.second.inet, font});
            }
            current->addItem(new astra::List(text),
                             new astra::CheckBox(item.second.up, [this, name = item.first](bool &checked) {
                                 enable_device(name, checked);
                             }));
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

bool
NetWorkDeviceListEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}