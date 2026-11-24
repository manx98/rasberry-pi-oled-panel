//
// Created by wenyiyu on 2024/10/28.
//

#include <thread>
#include <atomic>
#include <utility>
#include <exception>
#include <algorithm>
#include <memory>
#include <spdlog/fmt/fmt.h>
#include "panel/wifi_password_dialog.h"
#include "panel/wifi_groups_policy.h"
#include "utils/wifi_nmcli.h"
#include "utils/shutdown.h"
#include "text.h"
#include "wifi.h"
#include "utils/wifi.h"
#include "spdlog/spdlog.h"

static bool sortWifiInfoStrengthCb(const RPI::WifiInfo &a, const RPI::WifiInfo &b) {
    return a.strength > b.strength;
}

WifiScanListEvent::WifiScanListEvent(std::string device)
    : device_name_(std::move(device)) {}

namespace {
// A section label participates in list layout but is never selectable.
astra::List *newWifiSection(const std::string &label) {
    auto *row = new astra::List({0, {{label, astra::getUIConfig().mainFont}}});
    row->selectable = false;
    row->sectionHeader = true;
    return row;
}

void addWifiPlaceholder(astra::Menu *menu, const std::string &label) {
    auto *row = new astra::List({0, {{label, astra::getUIConfig().mainFont}}});
    row->selectable = false;
    menu->addItem(row);
}
} // namespace

void WifiScanListEvent::populate(astra::Menu *current) {
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    // A new scan/rebuilt saved list must not retain the previous viewport.
    current->initFlag = false;
    dynamic_cast<astra::List *>(current)->refreshBoundary(0, HAL::getSystemConfig().screenHeight);
    current->scrollSelectedTextOnly = true;
    const auto font = astra::getUIConfig().mainFont;
    const auto &ui = astra::getUIConfig();
    const float textWidth = HAL::getSystemConfig().screenWeight
                            - ui.listBarWeight - ui.checkBoxRightMargin
                            - ui.listTextMargin * 2;
    auto addScrollingItem = [&](const std::string &label, astra::List::Event *event) {
        astra::TextBox title{0, {{label, font}}};
        title.setMaxWidth(textWidth);
        current->addItem(new astra::List(title, event));
    };

    current->addItem(newWifiSection("已保存"));
    if (m_connections.empty()) {
        addWifiPlaceholder(current, "（暂无已保存网络）");
    } else {
        for (const auto &connection : m_connections) {
            const std::string name = connection.ssid.empty() ? connection.id : connection.ssid;
            addScrollingItem("- " + name,
                new WifiSavedProfileEvent(connection, device_name_, this));
        }
    }

    current->addItem(newWifiSection("附近网络"));
    bool anyNearby = false;
    for (const auto &item : m_networks) {
        // Saved networks already have their own section; do not display the
        // same access point twice. Fixed BSSID profiles are matched exactly.
        if (wifi_groups::isSavedNetwork(m_connections, item)) continue;
        anyNearby = true;
        addScrollingItem(fmt::format("[{:02d}] {}", item.strength,
            (item.security == RPI::WifiSecurity::Open ? "[开] " : "[密] ")
            + (item.SSID.empty() ? EMPTY_SSID : item.SSID)),
            new WifiOperationEvent(&m_connections, item, device_name_, this));
    }
    if (!anyNearby) addWifiPlaceholder(current, "（暂无附近网络）");

    // Section headers are non-selectable. Start on an actual access point or
    // saved profile; when both are empty, remain with no active highlight.
    for (std::size_t index = 0; index < current->childMenu.size(); ++index) {
        if (current->childMenu[index]->selectable) {
            current->selectIndex = static_cast<unsigned char>(index);
            break;
        }
    }
}

bool WifiScanListEvent::beforeOpen(astra::Menu *current) {
    astra::TextBox title(0, {{SCANNING_TEXT, astra::getUIConfig().mainFont}});
    std::atomic<bool> finished{false};
    std::list<RPI::WifiInfo> scanned;
    std::list<RPI::ConnectionInfo> saved;
    std::thread worker([&]() {
        try {
            saved = RPI::get_connections();
            scanned = RPI::getWifiList(device_name_);
            scanned.sort(sortWifiInfoStrengthCb);
        } catch (const std::exception &e) {
            spdlog::error("Wi-Fi scan failed: {}", e.what());
        }
        finished = true;
    });
    panel_shutdown::JoinThread join(worker);
    astra::Launcher::progress(HAL::getSystemConfig().screenWeight - 20, &title, nullptr,
        [&](astra::Clocker &, key::keyIndex) { return !finished.load(); });
    worker.join();
    saved.sort([](const auto &a, const auto &b) {
        const auto &nameA = a.ssid.empty() ? a.id : a.ssid;
        const auto &nameB = b.ssid.empty() ? b.id : b.ssid;
        return nameA == nameB ? a.uuid < b.uuid : nameA < nameB;
    });
    m_connections = std::move(saved);
    m_networks = std::move(scanned);
    refresh_requested_ = false;
    populate(current);
    return true;
}

bool WifiScanListEvent::beforeRender(astra::Menu *current,
                                     const std::vector<float> &camera,
                                     astra::Clocker &) {
    if (refresh_requested_) {
        refresh_requested_ = false;
        // Called when the list becomes visible again, not from inside a
        // child action: no event object is destroyed while executing itself.
        m_connections = RPI::get_connections();
        m_connections.sort([](const auto &a, const auto &b) {
            const auto &nameA = a.ssid.empty() ? a.id : a.ssid;
            const auto &nameB = b.ssid.empty() ? b.id : b.ssid;
            return nameA == nameB ? a.uuid < b.uuid : nameA < nameB;
        });
        populate(current);
        current->childPosInit(camera);
        astra::Launcher::getSelector()->inject(current);
    }
    return true;
}

bool WifiDeviceEvent::beforeOpen(astra::Menu *current) {
    std::atomic<bool> finished{false};
    std::list<RPI::NetWorkDeviceInfo> devices;
    // Saved profiles are fetched when a Wi-Fi device is opened.
    std::thread t([&]() {
        try {
            devices = RPI::getWifiDevices();
        } catch (const std::exception &e) {
            spdlog::error("Wi-Fi device lookup failed: {}", e.what());
        }
        finished = true;
    });
    panel_shutdown::JoinThread join(t);
    astra::TextBox title(0, {{LOADING_DEVICE_TEXT, astra::getUIConfig().mainFont}});
    astra::Launcher::progress(HAL::getSystemConfig().screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &clocker, key::keyIndex key) -> bool {
                                  return !finished;
                              });
    t.join();
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    auto font = astra::getUIConfig().mainFont;
    for (const auto &item : devices) {
        astra::TextBox title = {1, {{"- " + item.iface, font}}};
        if (!item.SSID.empty()) title.add({"  " + item.SSID, font});
        current->addItem(new astra::List(title, new WifiScanListEvent(item.iface)));
    }
    if (devices.empty()) {
        current->addItem(new astra::List({0, {{"No Wi-Fi devices", font}}}));
    }
    return true;
}

bool WifiDeviceEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

bool WifiOperationEvent::beforeOpen(astra::Menu *current) {
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    const auto font = astra::getUIConfig().mainFont;
    current->addItem(new astra::List({0, {
        {m_info.SSID.empty() ? EMPTY_SSID : m_info.SSID, font},
        {m_info.BSS, font},
        {fmt::format("[{:02d}] {} MHz", nm_utils_wifi_freq_to_channel(m_info.frequency),
                     m_info.frequency), font}
    }}));

    std::unique_ptr<RPI::ConnectionInfo> activeConnection(RPI::get_active_connection(m_device_name));
    bool active = false;
    if (activeConnection) {
        active = activeConnection->bssid.empty()
                 ? m_info.SSID == activeConnection->ssid
                 : m_info.BSS == activeConnection->bssid;
    }

    RPI::ConnectionInfo saved;
    bool hasSaved = false;
    if (m_connections) {
        for (const auto &connection : *m_connections) {
            const bool matching = connection.bssid.empty()
                                  ? connection.ssid == m_info.SSID
                                  : (connection.ssid == m_info.SSID && connection.bssid == m_info.BSS);
            if (matching) {
                saved = connection;
                hasSaved = true;
                break;
            }
        }
    }

    const std::string label = active ? DISCONNECT_WIFI_TEXT
        : (hasSaved ? "- 连接(已保存)" : CONNECT_WIFI_TEXT);
    current->addItem(new astra::List({0, {{label, font}}},
        new WifiOperationActionEvent(m_info, m_device_name, hasSaved ? saved.uuid : "", active, m_owner)));

    if (hasSaved) {
        current->addItem(new astra::List({0, {{FORGET_WIFI_TEXT, font}}},
                                             new WifiOperationForgetEvent(saved, m_owner)));
    }
    return true;
}

bool
WifiOperationEvent::beforeRender(astra::Menu *current, const std::vector<float> &_camera, astra::Clocker &clocker) {
    return true;
}

WifiOperationEvent::WifiOperationEvent(const std::list<RPI::ConnectionInfo> *connections,
                                       RPI::WifiInfo wifi, std::string device_name,
                                       WifiScanListEvent *owner) :
        m_connections(connections), m_owner(owner), m_info(std::move(wifi)),
        m_device_name(std::move(device_name)) {
}

namespace {

RPI::WifiCommandResult doWifiCommand(const std::function<RPI::WifiCommandResult()> &operation) {
    std::atomic<bool> done{false};
    RPI::WifiCommandResult result = RPI::WifiCommandResult::Failed;
    std::thread worker([&]() {
        try { result = operation(); }
        catch (const std::exception &e) {
            // Do not log the password or network credentials.
            spdlog::warn("Wi-Fi command failed: {}", e.what());
        }
        done.store(true);
    });
    panel_shutdown::JoinThread join(worker);
    astra::TextBox title(0, {{"正在处理WiFi...", astra::getUIConfig().mainFont}});
    astra::Launcher::progress(HAL::getSystemConfig().screenWeight - 20, &title, nullptr,
                              [&](astra::Clocker &, key::keyIndex) { return !done.load(); });
    worker.join();
    return result;
}

void wifiNotice(const std::string &message) {
    astra::Launcher::popInfo({0, {{message, astra::getUIConfig().mainFont}}}, 1300);
}

void erasePassword(std::string &password) {
    volatile char *bytes = password.empty() ? nullptr : &password[0];
    for (std::size_t i = 0; bytes && i < password.size(); ++i) bytes[i] = 0;
    password.clear();
}

} // namespace

WifiOperationActionEvent::WifiOperationActionEvent(RPI::WifiInfo info, std::string device,
                                                   std::string savedConnectionUuid,
                                                   bool disconnect, WifiScanListEvent *owner)
    : info_(std::move(info)), device_(std::move(device)),
      savedConnectionId_(std::move(savedConnectionUuid)), disconnect_(disconnect),
      owner_(owner) {}

bool WifiOperationActionEvent::beforeOpen(astra::Menu *current) {
    const auto font = astra::getUIConfig().mainFont;
    if (disconnect_ && !confirmWifiOperation(info_.SSID, "断开")) return false;
    if (!disconnect_ && savedConnectionId_.empty()
        && info_.security == RPI::WifiSecurity::Enterprise) {
        wifiNotice("不支持企业WiFi认证");
        return false;
    }
    std::string password;
    if (!disconnect_ && savedConnectionId_.empty()
        && info_.security != RPI::WifiSecurity::Open) {
        if (!inputWifiPassword(info_.SSID,
                               info_.security == RPI::WifiSecurity::LegacyWep, password)) return false;
    }

    const auto result = doWifiCommand([&]() {
        if (disconnect_) return RPI::wifiDisconnect(device_);
        if (!savedConnectionId_.empty()) return RPI::wifiConnectSaved(device_, savedConnectionId_);
        return RPI::wifiConnectNew(device_, info_.SSID, info_.BSS, password);
    });
    erasePassword(password);

    if (result == RPI::WifiCommandResult::Success) {
        if (owner_) owner_->requestRefresh();
        disconnect_ = !disconnect_;
        current->title.setText(0, {disconnect_ ? DISCONNECT_WIFI_TEXT : CONNECT_WIFI_TEXT, font});
        wifiNotice(disconnect_ ? "WiFi已连接" : "WiFi已断开");
    } else if (result == RPI::WifiCommandResult::Timeout) {
        wifiNotice("WiFi操作超时");
    } else if (result == RPI::WifiCommandResult::Unavailable) {
        wifiNotice("无法启动WiFi连接工具");
    } else {
        wifiNotice(disconnect_ ? "WiFi断开失败" : "WiFi连接失败 请检查密码");
    }
    return false; // Action, not a submenu.
}

WifiOperationForgetEvent::WifiOperationForgetEvent(RPI::ConnectionInfo info,
                                                   WifiScanListEvent *owner)
    : m_info(std::move(info)), owner_(owner) {}

bool WifiOperationForgetEvent::beforeOpen(astra::Menu *current) {
    if (m_info.uuid.empty()) {
        wifiNotice("找不到已保存连接");
        return false;
    }
    if (!confirmWifiOperation(m_info.ssid, "忘记")) return false;
    const auto result = doWifiCommand([&]() { return RPI::wifiForgetSaved(m_info.uuid); });
    if (result == RPI::WifiCommandResult::Success) {
        // The popup renders the current menu. Show it before scheduling
        // deletion, otherwise the delayed-removal list could delete this
        // event object while beforeOpen() is still on the stack.
        wifiNotice("WiFi配置已删除");
        if (owner_) owner_->requestRefresh();
        if (current->parent) current->parent->removeItem(current);
    } else wifiNotice("忘记WiFi失败");
    return false;
}

bool WifiOperationForgetEvent::beforeRender(astra::Menu *, const std::vector<float> &,
                                            astra::Clocker &) {
    return false;
}

WifiSavedProfileEvent::WifiSavedProfileEvent(RPI::ConnectionInfo saved, std::string device,
                                             WifiScanListEvent *owner)
    : saved_(std::move(saved)), device_(std::move(device)), owner_(owner) {}

bool WifiSavedProfileEvent::beforeOpen(astra::Menu *current) {
    current->clear();
    current->selectIndex = 0;
    current->resetCameraMemoryPos();
    const auto font = astra::getUIConfig().mainFont;
    const std::string name = saved_.ssid.empty() ? saved_.id : saved_.ssid;
    auto *info = new astra::List({0, {{name, font}}});
    info->selectable = false; // SSID is a heading, not a third action.
    info->sectionHeader = true;
    current->addItem(info);
    current->addItem(new astra::List({0, {{CONNECT_WIFI_TEXT, font}}},
        new WifiSavedActionEvent(saved_, device_, owner_, false)));
    current->addItem(new astra::List({0, {{FORGET_WIFI_TEXT, font}}},
        new WifiSavedActionEvent(saved_, device_, owner_, true)));
    current->selectIndex = 1;
    return true;
}

WifiSavedActionEvent::WifiSavedActionEvent(RPI::ConnectionInfo saved, std::string device,
                                           WifiScanListEvent *owner, bool forget)
    : saved_(std::move(saved)), device_(std::move(device)), owner_(owner), forget_(forget) {}

bool WifiSavedActionEvent::beforeOpen(astra::Menu *) {
    const std::string name = saved_.ssid.empty() ? saved_.id : saved_.ssid;
    if (saved_.uuid.empty()) {
        wifiNotice("找不到已保存连接");
        return false;
    }
    if (forget_) {
        // Forgetting the SSH-carrying profile can disrupt the connection.
        // Never do it without a separate confirmation (default: cancel).
        if (!confirmWifiOperation(name, "忘记")) return false;
        const auto result = doWifiCommand([&]() {
            return RPI::wifiForgetSaved(saved_.uuid);
        });
        if (result == RPI::WifiCommandResult::Success) {
            if (owner_) owner_->requestRefresh();
            wifiNotice("WiFi配置已删除");
            // Return to the Wi-Fi list; the old profile menu is discarded on
            // its next render, after the active action handler has returned.
            astra::Launcher::close();
        } else if (result == RPI::WifiCommandResult::Timeout) {
            wifiNotice("忘记WiFi超时");
        } else {
            wifiNotice("忘记WiFi失败");
        }
        return false;
    }

    // Selecting "连接" for the currently connected profile is a no-op.
    // Do not risk needlessly interrupting an active network or remote SSH.
    std::unique_ptr<RPI::ConnectionInfo> active(RPI::get_active_connection(device_));
    if (active && active->uuid == saved_.uuid) {
        wifiNotice("WiFi已连接");
        return false;
    }
    const auto result = doWifiCommand([&]() {
        return RPI::wifiConnectSaved(device_, saved_.uuid);
    });
    if (result == RPI::WifiCommandResult::Success) {
        if (owner_) owner_->requestRefresh();
        wifiNotice("WiFi已连接");
    } else if (result == RPI::WifiCommandResult::Timeout) {
        wifiNotice("WiFi连接超时");
    } else {
        wifiNotice("WiFi连接失败");
    }
    return false;
}
