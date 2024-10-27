//
// Created by wenyiyu on 2024/10/17.
//

#ifndef RASPBERRY_PI_OLED_PANEL_WIFI_H
#define RASPBERRY_PI_OLED_PANEL_WIFI_H
#include <libnm/NetworkManager.h>
#include <list>
#include <string>
#include <functional>

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

    /* nmcli exit codes */
    enum NMCResultCode {
        /* Indicates successful execution */
        NMC_RESULT_SUCCESS = 0,

        /* Unknown / unspecified error */
        NMC_RESULT_ERROR_UNKNOWN = 1,

        /* Wrong invocation of nmcli */
        NMC_RESULT_ERROR_USER_INPUT = 2,

        /* A timeout expired */
        NMC_RESULT_ERROR_TIMEOUT_EXPIRED = 3,

        /* Error in connection activation */
        NMC_RESULT_ERROR_CON_ACTIVATION = 4,

        /* Error in connection deactivation */
        NMC_RESULT_ERROR_CON_DEACTIVATION = 5,

        /* Error in device disconnect */
        NMC_RESULT_ERROR_DEV_DISCONNECT = 6,

        /* Error in connection deletion */
        NMC_RESULT_ERROR_CON_DEL = 7,

        /* NetworkManager is not running */
        NMC_RESULT_ERROR_NM_NOT_RUNNING = 8,

        /* No more used, keep to preserve API */
        NMC_RESULT_ERROR_VERSIONS_MISMATCH = 9,

        /* Connection/Device/AP not found */
        NMC_RESULT_ERROR_NOT_FOUND = 10,

        /* --complete-args signals a file name may follow */
        NMC_RESULT_COMPLETE_FILE = 65,

        /*Connect need password*/
        NMC_RESULT_ERROR_CON_REQUIRE_SECRET = 66,
    };

    NMCResultCode exist_connect_wifi_ap(const std::string &interface_name, const std::string& bssid);

    NMCResultCode connect_wifi_ap(const std::string &interface_name, const std::string& bssid, const std::string& password, std::function<void(const char*)> state_callback);

    NMCResultCode do_device_wifi_hotspot(const std::string &interface_name, const std::string& ssid, const std::string& password);

    NMCResultCode do_connection_up(const std::string &interface_name, const std::string& conn_id);

    NMCResultCode do_connection_down(const std::string &interface_name, const std::string& conn_id);

    NMCResultCode do_connection_delete(const std::string &interface_name, const std::string &conn_id);

    NMConnection* find_connection_by_id(NMDevice *device, const char *id);

    struct ConnectionInfo {
        std::string id;
        std::string bssid;
        std::string ssid;
    };

    std::list<ConnectionInfo> get_connections();

    ConnectionInfo* get_active_connection(const std::string &interface_name);

    std::string gBytesToStr(GBytes *bytes);

    std::string cStr(const char* str);
}

#endif //RASPBERRY_PI_OLED_PANEL_WIFI_H
