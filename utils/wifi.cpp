//
// Created by wenyiyu on 2024/10/17.
//

#include "wifi.h"
#include "nm-shared-utils.h"
#include <spdlog/spdlog.h>

namespace RPI {
    struct WifiScanData {
        std::list<WifiInfo> result;
        GMainLoop *loop;
        GError *error;
        gint64 rescan_cutoff_msec;
        NMDeviceWifi *wifi;
        gulong last_scan_id;
        guint timeout_id;
        GCancellable *scan_cancellable;
    };

    static inline gboolean
    nm_clear_g_source(guint *id) {
        guint v;

        if (id && (v = *id)) {
            *id = 0;
            g_source_remove(v);
            return TRUE;
        }
        return FALSE;
    }

    static inline gboolean
    nm_clear_g_cancellable(GCancellable * *cancellable) {
        GCancellable * v;

        if (cancellable && (v = *cancellable)) {
            *cancellable = nullptr;
            g_cancellable_cancel(v);
            g_object_unref(v);
            return TRUE;
        }
        return FALSE;
    }

    static gint64
    _device_wifi_get_last_scan(NMDeviceWifi *wifi) {
        gint64 timestamp;

        timestamp = nm_device_wifi_get_last_scan(wifi);
        if (timestamp == -1)
            return G_MININT64;
        return timestamp;
    }

    static void wifi_list_finish(WifiScanData *data, bool force_finished) {
        if (!force_finished && data->rescan_cutoff_msec > _device_wifi_get_last_scan(data->wifi)) {
            return;
        }
        nm_clear_g_signal_handler(data->wifi, &data->last_scan_id);
        nm_clear_g_source(&data->timeout_id);
        nm_clear_g_cancellable(&data->scan_cancellable);
        auto access_points = (GPtrArray *) nm_device_wifi_get_access_points(data->wifi);
        if (access_points) {
            for (guint i = 0; i < access_points->len; i++) {
                NMAccessPoint *ap = NM_ACCESS_POINT(g_ptr_array_index(access_points, i));

                WifiInfo info;
                // 获取 Wi-Fi SSID
                const GByteArray *ssid = (GByteArray *) nm_access_point_get_ssid(ap);
                if (ssid && ssid->len > 0) {
                    info.SSID = std::string((const char *) ssid->data, ssid->len);
                }
                info.BSS = nm_access_point_get_bssid(ap);
                // 获取信号强度和频率
                info.strength = nm_access_point_get_strength(ap);
                info.frequency = nm_access_point_get_frequency(ap);
                spdlog::info("SSID: {}, BSSID: {}, Strength: {}, Frequency: {}", info.SSID, info.BSS, info.strength,
                             info.frequency);
                data->result.emplace_back(info);
            }
        }
        g_main_loop_quit(data->loop);
    }

    static gboolean wifi_list_scan_timeout(gpointer user_data) {
        auto wifi_list_data = (WifiScanData *) user_data;

        wifi_list_data->timeout_id = 0;
        wifi_list_finish(wifi_list_data, TRUE);
        return G_SOURCE_REMOVE;
    }

    static void on_wifi_scan_done(GObject *client, GAsyncResult *result, gpointer user_data) {
        auto data = (WifiScanData *) user_data;
        if (nm_device_wifi_request_scan_finish(data->wifi, result, &data->error)) {
            wifi_list_finish(data, false);
        } else {
            wifi_list_finish(data, true);
        }
    }

    static void
    wifi_last_scan_updated(GObject *gobject, GParamSpec *pspec, gpointer user_data)
    {
        wifi_list_finish((WifiScanData*)user_data, FALSE);
    }

    static inline gboolean
    nm_utils_error_is_cancelled(GError *error)
    {
        return error && error->code == G_IO_ERROR_CANCELLED && error->domain == G_IO_ERROR;
    }

    static void
    wifi_list_rescan_retry_cb(gpointer user_data, GCancellable *cancellable)
    {
        auto wifi_list_data = (WifiScanData *) user_data;

        if (g_cancellable_is_cancelled(cancellable))
            return;
        g_clear_error(&wifi_list_data->error);
        nm_device_wifi_request_scan_async(wifi_list_data->wifi,
                                          wifi_list_data->scan_cancellable,
                                          wifi_list_rescan_cb,
                                          wifi_list_data);
    }

    //  #define NM_DEVICE_ERROR nm_device_error_quark()
    //  NM_CACHED_QUARK_FCN("nm-device-error-quark", nm_device_error_quark);
    static GQuark my_nm_device_error_quark = g_quark_from_static_string("nm-device-error-quark");

    void
    wifi_list_rescan_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
    {
        NMDeviceWifi         *wifi  = NM_DEVICE_WIFI(source_object);
        auto wifi_list_data = (WifiScanData *)user_data;
        gboolean              force_finished;
        gboolean              done;

        nm_device_wifi_request_scan_finish(wifi, res, &wifi_list_data->error);
        if (nm_utils_error_is_cancelled(wifi_list_data->error))
            return;
        if (g_error_matches(wifi_list_data->error, my_nm_device_error_quark, NM_DEVICE_ERROR_NOT_ALLOWED)) {
            if (nm_device_get_state(NM_DEVICE(wifi)) < NM_DEVICE_STATE_DISCONNECTED) {
                /* the device is either unmanaged or unavailable.
                 *
                 * If it's unmanaged, we don't expect any scan result and are done.
                 * If it's unavailable, that usually means that we wait for wpa_supplicant
                 * to start. In that case, also quit (without scan results). */
                force_finished = TRUE;
                done           = TRUE;
            } else {
                /* This likely means that scanning is already in progress. There's
                 * a good chance we'll get updated results soon; wait for them.
                 *
                 * But also, NetworkManager ratelimits (and rejects requests). That
                 * means, possibly we were just ratelimited, so waiting will not lead
                 * to a new scan result. Instead, repeatedly ask new scans... */
                nm_utils_invoke_on_timeout(1000,
                                           wifi_list_data->scan_cancellable,
                                           wifi_list_rescan_retry_cb,
                                           wifi_list_data);
                force_finished = FALSE;
                done           = FALSE;
            }
        } else if (wifi_list_data->error) {
            force_finished = TRUE;
            done           = TRUE;
        } else {
            force_finished = FALSE;
            done           = TRUE;
        }

        if (done)
            g_clear_object(&wifi_list_data->scan_cancellable);
        wifi_list_finish(wifi_list_data, force_finished);
    }

    std::list<WifiInfo> getWifiList(const std::string &interface_name) {
        GError * error = nullptr;
        // 创建 NetworkManager 客户端
        NMClient *client = nm_client_new(nullptr, &error);
        if (error) {
            spdlog::error("Error creating NMClient: {}", error->message);
            g_error_free(error);
            return {};
        }

        auto device = nm_client_get_device_by_iface(client, interface_name.c_str());
        if (!device) {
            spdlog::error("Device {} not found", interface_name);
            g_object_unref(client);
            return {};
        }
        // 检查设备是否为 Wi-Fi 设备
        if (!NM_IS_DEVICE_WIFI(device)) {
            spdlog::error("Device {} is not a Wi-Fi device", interface_name);
            g_object_unref(client);
            g_object_unref(device);
            return {};
        }
        WifiScanData data{
                .loop = g_main_loop_new(nullptr, FALSE),
                .error = nullptr,
                .rescan_cutoff_msec = nm_utils_get_timestamp_msec(),
                .wifi = NM_DEVICE_WIFI(device),
        };
        auto timeout_msec = data.rescan_cutoff_msec <= _device_wifi_get_last_scan(data.wifi) ? 0 : 15000;
        data.timeout_id = g_timeout_add(timeout_msec, wifi_list_scan_timeout, &data);
        if (timeout_msec > 0) {
            data.last_scan_id = g_signal_connect(data.wifi,
                                                 "notify::" NM_DEVICE_WIFI_LAST_SCAN,
                                                 G_CALLBACK(wifi_last_scan_updated),
                                                 &data),
            nm_device_wifi_request_scan_async(data.wifi, nullptr, on_wifi_scan_done, &data);
            data.scan_cancellable = g_cancellable_new(),
            nm_device_wifi_request_scan_async(data.wifi,
                                              data.scan_cancellable,
                                              wifi_list_rescan_cb,
                                              &data);
        }
        g_main_loop_run(data.loop);
        g_main_loop_unref(data.loop);
        g_object_unref(client);
        if(data.error) {
            spdlog::error("Error getting wifi list: {}", data.error->message);
            g_error_free(data.error);
            return {};
        }
        return data.result;
    }

    std::list<NetWorkDeviceInfo> getWifiDevices() {
        GError * err = nullptr;
        auto client = nm_client_new(nullptr, &err);
        if (!client) {
            spdlog::warn("Failed to create NMClient: {}", err->message);
            g_error_free(err);
            return {};
        }
        // 获取所有设备
        auto devices = nm_client_get_all_devices (client);
        if (!devices) {
            spdlog::warn("Failed to get devices");
            g_object_unref(client);
            return {};
        }
        std::list<NetWorkDeviceInfo> result;
        // 遍历所有设备并打印信息
        for (auto i = 0; i < devices->len; i++) {
            NetWorkDeviceInfo info;
            auto *device = (NMDevice *) g_ptr_array_index(devices, i);
            if (!NM_IS_DEVICE_WIFI(device)) {
                continue;
            }

            // 获取设备状态
            info.state = nm_device_get_state(device);

            // 获取设备唯一标识符
            info.uid = nm_device_get_udi(device);

            // 获取设备接口名称
            info.iface = nm_device_get_iface(device);

            // 获取连接信息
            NMActiveConnection *active_connection = nm_device_get_active_connection(device);
            if (active_connection) {
                auto connection = (NMConnection *) nm_active_connection_get_connection(active_connection);
                if (connection) {
                    auto setting_wireless = nm_connection_get_setting_wireless(connection);
                    gsize ssid_len;
                    auto ssid_data = g_bytes_get_data(nm_setting_wireless_get_ssid(setting_wireless), &ssid_len);
                    info.SSID = std::string((const char *) ssid_data, ssid_len);
                }
            }

            spdlog::info("type:{} state:{} udi:{} iface:{} SSID:{}", nm_device_get_type_description(device),
                         int(info.state), info.uid, info.iface, info.SSID);
            result.emplace_back(info);
        }
        g_object_unref(client);
        return result;
    }

    const char *
    nmc_wifi_strength_bars(guint8 strength)
    {
        if (strength > 80)
            return /* ▂▄▆█ */ "\342\226\202\342\226\204\342\226\206\342\226\210";
        else if (strength > 55)
            return /* ▂▄▆_ */ "\342\226\202\342\226\204\342\226\206_";
        else if (strength > 30)
            return /* ▂▄__ */ "\342\226\202\342\226\204__";
        else if (strength > 5)
            return /* ▂___ */ "\342\226\202___";
        else
            return /* ____ */ "____";
    }

    WifiConnectState
    connectWifi(const std::string &interface_name, const std::string &ssid, const std::string &password) {
        GError * error = nullptr;
        // 创建 NetworkManager 客户端
        NMClient *client = nm_client_new(nullptr, &error);
        if (error) {
            spdlog::error("Error creating NMClient: {}", error->message);
            g_error_free(error);
            return {};
        }

        auto device = nm_client_get_device_by_iface(client, interface_name.c_str());
        if (!device) {
            spdlog::error("Device {} not found", interface_name);
            g_object_unref(client);
            return {};
        }
        g_object_unref(device);
        g_object_unref(client);
        return WIFI_Failed;
    }
}