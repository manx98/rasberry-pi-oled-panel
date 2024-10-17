//
// Created by wenyiyu on 2024/10/17.
//

#ifndef RASPBERRY_PI_OLED_PANEL_WIFI_H
#define RASPBERRY_PI_OLED_PANEL_WIFI_H
#include <libnm/NetworkManager.h>
#include <list>
#include <string>

namespace RPI {
    struct NetWorkDeviceInfo {
        std::string iface;
        NMDeviceState state;
        std::string uid;
        std::string SSID;
    };
    struct WifiInfo {
        std::string BSS;
        std::string SSID;
        guint8 strength;
        guint32 frequency;
    };
    void wifi_list_rescan_cb(GObject *source_object, GAsyncResult *res, gpointer user_data);

    std::list<NetWorkDeviceInfo> getWifiDevices();

    const char *
    nmc_wifi_strength_bars(guint8 strength);

    std::list<WifiInfo> getWifiList(const std::string &interface_name);

    enum WifiConnectState {
        WIFI_Failed,
        WIFI_Connecting,
        WIFI_Connected
    };

    WifiConnectState connectWifi(const std::string &interface_name, const std::string &ssid, const std::string &password);
}

#endif //RASPBERRY_PI_OLED_PANEL_WIFI_H
