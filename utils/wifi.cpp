//
// Created by wenyiyu on 2024/10/17.
//

#include "wifi.h"
#include "nm-shared-utils.h"
#include "nm-utils.h"
#include <spdlog/spdlog.h>

#include <utility>

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
    nm_clear_g_cancellable(GCancellable **cancellable) {
        GCancellable *v;

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
                if (!ap) {
                    continue;
                }
                WifiInfo info;
                // 获取 Wi-Fi SSID
                info.SSID = gBytesToStr(nm_access_point_get_ssid(ap));
                info.BSS = cStr(nm_access_point_get_bssid(ap));
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
    wifi_last_scan_updated(GObject *gobject, GParamSpec *pspec, gpointer user_data) {
        wifi_list_finish((WifiScanData *) user_data, FALSE);
    }

    static inline gboolean
    nm_utils_error_is_cancelled(GError *error) {
        return error && error->code == G_IO_ERROR_CANCELLED && error->domain == G_IO_ERROR;
    }

    static void
    wifi_list_rescan_retry_cb(gpointer user_data, GCancellable *cancellable) {
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
    wifi_list_rescan_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
        NMDeviceWifi *wifi = NM_DEVICE_WIFI(source_object);
        auto wifi_list_data = (WifiScanData *) user_data;
        gboolean force_finished;
        gboolean done;

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
                done = TRUE;
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
                done = FALSE;
            }
        } else if (wifi_list_data->error) {
            force_finished = TRUE;
            done = TRUE;
        } else {
            force_finished = FALSE;
            done = TRUE;
        }

        if (done)
            g_clear_object(&wifi_list_data->scan_cancellable);
        wifi_list_finish(wifi_list_data, force_finished);
    }

    static std::list<WifiInfo> getWifiList(NMDevice *device) {
        // 检查设备是否为 Wi-Fi 设备
        if (!NM_IS_DEVICE_WIFI(device)) {
            spdlog::error("Device {} is not a Wi-Fi device", nm_device_get_iface(device));
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
                                                 &data);
            nm_device_wifi_request_scan_async(data.wifi, nullptr, on_wifi_scan_done, &data);
            data.scan_cancellable = g_cancellable_new();
            nm_device_wifi_request_scan_async(data.wifi,
                                              data.scan_cancellable,
                                              wifi_list_rescan_cb,
                                              &data);
        }
        g_main_loop_run(data.loop);
        g_main_loop_unref(data.loop);
        if (data.error) {
            spdlog::error("Error getting wifi list: {}", data.error->message);
            g_error_free(data.error);
            return {};
        }
        return data.result;
    }

    std::list<WifiInfo> getWifiList(const std::string &interface_name) {
        GError *error = nullptr;
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
        auto ret = getWifiList(device);
        g_object_unref(client);
        return ret;
    }

    std::list<NetWorkDeviceInfo> getWifiDevices() {
        GError *err = nullptr;
        auto client = nm_client_new(nullptr, &err);
        if (!client) {
            spdlog::warn("Failed to create NMClient: {}", err->message);
            g_error_free(err);
            return {};
        }
        // 获取所有设备
        auto devices = nm_client_get_all_devices(client);
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
            info.uid = cStr(nm_device_get_udi(device));

            // 获取设备接口名称
            info.iface = cStr(nm_device_get_iface(device));

            // 获取连接信息
            NMActiveConnection *active_connection = nm_device_get_active_connection(device);
            if (active_connection) {
                auto connection = (NMConnection *) nm_active_connection_get_connection(active_connection);
                if (connection) {
                    auto setting_wireless = nm_connection_get_setting_wireless(connection);
                    if (!setting_wireless) {
                        continue;
                    }
                    info.SSID = gBytesToStr(nm_setting_wireless_get_ssid(setting_wireless));
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
    nmc_wifi_strength_bars(guint8 strength) {
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

    /*
     * Find AP on 'device' according to 'bssid' and 'ssid' parameters.
     * Returns: found AP or NULL
     */
    static NMAccessPoint *
    find_ap_on_device(NMDevice *device, const char *bssid, const char *ssid, bool complete) {
        const GPtrArray *aps;
        NMAccessPoint   *ap = NULL;
        int              i;

        g_return_val_if_fail(NM_IS_DEVICE_WIFI(device), NULL);
        getWifiList(device);
        aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));
        for (i = 0; i < aps->len; i++) {
            auto candidate_ap = (NMAccessPoint*) g_ptr_array_index(aps, i);

            if (bssid) {
                const char *candidate_bssid = nm_access_point_get_bssid(candidate_ap);

                if (!candidate_bssid)
                    continue;

                /* Compare BSSIDs */
                if (complete) {
                    if (g_str_has_prefix(candidate_bssid, bssid))
                        g_print("%s\n", candidate_bssid);
                } else if (strcmp(bssid, candidate_bssid) != 0)
                    continue;
            }

            if (ssid) {
                /* Parameter is SSID */
                GBytes *candidate_ssid;
                char   *ssid_tmp;

                candidate_ssid = nm_access_point_get_ssid(candidate_ap);
                if (!candidate_ssid)
                    continue;

                ssid_tmp = nm_utils_ssid_to_utf8((const guint8 *)g_bytes_get_data(candidate_ssid, nullptr),
                                                 g_bytes_get_size(candidate_ssid));

                /* Compare SSIDs */
                if (complete) {
                    if (g_str_has_prefix(ssid_tmp, ssid))
                        g_print("%s\n", ssid_tmp);
                } else if (strcmp(ssid, ssid_tmp) != 0) {
                    g_free(ssid_tmp);
                    continue;
                }
                g_free(ssid_tmp);
            }

            if (complete)
                continue;

            ap = candidate_ap;
            break;
        }

        return ap;
    }

    struct WifiConnectData {
        NMClient *client{nullptr};
        NMDevice *device{nullptr};
        std::string return_text;
        NMCResultCode return_value;
        GMainLoop *loop{nullptr};
        gboolean create;
        gboolean hotspot;
        NMConnection *connection;
        NMActiveConnection *active{nullptr};
        guint progress_id{0};
        std::function<void(const char *)> state_callback;
        const char *specific_object;
        /* Operation timeout */
        int timeout{90};
        bool running{true};

        void quite() {
            if(running) {
                if(loop) {
                    g_main_loop_quit(loop);
                }
                if(progress_id) {
                    nm_clear_g_source(&progress_id);
                }
                running = false;
            }
        }
    };

    const char *
    nmc_device_state_to_string_with_external(NMDevice *device) {
        NMActiveConnection *ac;
        NMDeviceState state;
        const char *s;

        state = nm_device_get_state(device);
        return nmc_device_state_to_string(state);
    }

    static gboolean
    progress_cb(gpointer user_data) {
        auto info = (WifiConnectData *) (user_data);
        info->state_callback(nmc_device_state_to_string_with_external(info->device));
        return TRUE;
    }

    const char *
    nmc_connection_check_deprecated(NMConnection *c) {
        NMSettingWirelessSecurity *s_wsec;
        const char *key_mgmt;
        const char *type;

        type = nm_connection_get_connection_type(c);
        if (nm_streq0(type, NM_SETTING_WIMAX_SETTING_NAME))
            return "WiMax is no longer supported";

        s_wsec = nm_connection_get_setting_wireless_security(c);
        if (s_wsec) {
            key_mgmt = nm_setting_wireless_security_get_key_mgmt(s_wsec);
            if (NM_IN_STRSET(key_mgmt, "ieee8021x", "none"))
                return "WEP encryption is known to be insecure";
        }

        return nullptr;
    }

    static void
    add_and_activate_check_state(WifiConnectData *info) {
        NMDeviceState state;
        NMDeviceStateReason reason;
        NMActiveConnectionState ac_state;

        state = nm_device_get_state(info->device);
        ac_state = nm_active_connection_get_state(info->active);

        if (ac_state == NM_ACTIVE_CONNECTION_STATE_ACTIVATING)
            return;

        if (state == NM_DEVICE_STATE_ACTIVATED) {
            spdlog::info("Device '{}' successfully activated with '{}'.",
                         nm_device_get_iface(info->device),
                         nm_active_connection_get_uuid(info->active));

            if (info->hotspot)
                spdlog::info("Hint: \"nmcli dev wifi show-password\" shows the Wi-Fi name and password.\n");
        } else if (state <= NM_DEVICE_STATE_DISCONNECTED || state >= NM_DEVICE_STATE_DEACTIVATING) {
            reason = nm_device_get_state_reason(info->device);
            info->return_text = fmt::format("Error: Connection activation failed: {}.",
                                            nmc_device_reason_to_string(reason));
            if (reason == NM_DEVICE_STATE_REASON_NO_SECRETS) {
                info->return_value = NMC_RESULT_ERROR_CON_REQUIRE_SECRET;
            } else {
                info->return_value = NMC_RESULT_ERROR_CON_ACTIVATION;
            }
        } else {
            return;
        }
        info->quite();
    }


    static void
    add_and_activate_notify_state_cb(GObject *src, GParamSpec *pspec, gpointer user_data) {
        add_and_activate_check_state((WifiConnectData *) user_data);
    }

    static gboolean
    timeout_cb(gpointer user_data) {
        /* Time expired -> exit nmcli */

        auto info = (WifiConnectData *) user_data;
        info->return_text = fmt::format("Error: Timeout %d sec expired.", info->timeout);
        info->return_value = NMC_RESULT_ERROR_TIMEOUT_EXPIRED;
        info->quite();
        return FALSE;
    }

    static void
    add_and_activate_cb(GObject *client, GAsyncResult *result, gpointer user_data) {
        auto info = (WifiConnectData *) user_data;
        NMActiveConnection *active = nullptr;
        const char *deprecated;
        GError *error = nullptr;
        if (info->create)
            active = nm_client_add_and_activate_connection_finish(NM_CLIENT(client), result, &error);
        else
            active = nm_client_activate_connection_finish(NM_CLIENT(client), result, &error);

        if (error) {
            if (info->hotspot) {
                info->return_text = fmt::format("Error: Failed to setup a Wi-Fi hotspot: {}", error->message);
            } else if (info->create) {
                info->return_text = fmt::format("Error: Failed to add/activate new connection: : {}", error->message);
            } else {
                info->return_text = fmt::format("Error: Failed to activate connection: {}", error->message);
            }
            info->return_value = NMC_RESULT_ERROR_CON_ACTIVATION;
            info->quite();
            g_error_free(error);
            return;
        }

        deprecated = nmc_connection_check_deprecated(NM_CONNECTION(nm_active_connection_get_connection(active)));
        if (deprecated)
            spdlog::warn("Warning: {}.", deprecated);

        if (info->state_callback) {
            info->progress_id = g_timeout_add(120, progress_cb, info);
        }

        info->active = (NMActiveConnection *)g_steal_pointer(&active);

        g_signal_connect(info->device,
                         "notify::" NM_DEVICE_STATE,
                         G_CALLBACK(add_and_activate_notify_state_cb),
                         info);

        g_signal_connect(info->active,
                         "notify::" NM_ACTIVE_CONNECTION_STATE,
                         G_CALLBACK(add_and_activate_notify_state_cb),
                         info);

        add_and_activate_check_state(info);

        g_timeout_add_seconds(info->timeout, timeout_cb, info); /* Exit if timeout expires */
    }


    static void
    activate_update2_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
        auto remote_con = NM_REMOTE_CONNECTION(source_object);
        auto info = (WifiConnectData *) user_data;
        GError *error = nullptr;
        auto ret = nm_remote_connection_update2_finish(remote_con, res, &error);
        if (!ret) {
            info->return_text = fmt::format("Error: {}.", error->message);
            info->return_value = NMC_RESULT_ERROR_UNKNOWN;
            g_error_free(error);
            info->quite();
            return;
        }

        nm_client_activate_connection_async(info->client,
                                            NM_CONNECTION(remote_con),
                                            info->device,
                                            info->specific_object,
                                            nullptr,
                                            add_and_activate_cb,
                                            info);
    }

    static void
    save_and_activate_connection(WifiConnectData *data) {
        if (NM_IS_REMOTE_CONNECTION(data->connection)) {
            nm_remote_connection_update2(NM_REMOTE_CONNECTION(data->connection),
                                         nm_connection_to_dbus(data->connection, NM_CONNECTION_SERIALIZE_ALL),
                                         NM_SETTINGS_UPDATE2_FLAG_BLOCK_AUTOCONNECT,
                                         nullptr,
                                         nullptr,
                                         activate_update2_cb,
                                         data);
        } else {
            nm_client_add_and_activate_connection_async(data->client,
                                                        data->connection,
                                                        data->device,
                                                        data->specific_object,
                                                        nullptr,
                                                        add_and_activate_cb,
                                                        data);
        }
    }

    static inline GVariant *
    nm_g_variant_new_ay(const guint8 *data, gsize len)
    {
        return g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, data, len, 1);
    }

    NMCResultCode
    do_device_wifi_connect_network(const std::string &interface_name, bool hidden, const char* bssid, const char* ssid, const std::string &password,
                                   std::function<void(const char *)> state_callback) {
        GError *error = nullptr;
        NMAccessPoint *ap;
        NMSettingWireless *s_wifi;
        const GPtrArray *avail_cons;
        NM80211ApFlags ap_flags;
        NM80211ApSecurityFlags ap_wpa_flags;
        NM80211ApSecurityFlags ap_rsn_flags;
        GBytes *g_ssid = nullptr;
        GBytes *bssid_bytes = nullptr;
        std::string str_ssid;
        NMSettingConnection* s_con;
        // 创建 NetworkManager 客户端
        NMClient *client = nm_client_new(nullptr, &error);
        WifiConnectData info{
                .client = client,
                .return_value = NMC_RESULT_SUCCESS,
                .state_callback = std::move(state_callback),
        };
        if (error) {
            spdlog::error("Error creating NMClient: {}", error->message);
            info.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }

        info.device = nm_client_get_device_by_iface(client, interface_name.c_str());
        if (!info.device) {
            spdlog::error("Device {} not found", interface_name);
            info.return_value = NMC_RESULT_ERROR_NOT_FOUND;
            goto out;
        }

        /* For hidden SSID first scan it so that NM learns about the AP */
        if (hidden) {
            GVariantBuilder builder, array_builder;
            GVariant       *options;
            GError         *scan_err = nullptr;

            g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
            g_variant_builder_init(&array_builder, G_VARIANT_TYPE("aay"));
            g_variant_builder_add(&array_builder,
                                  "@ay",
                                  nm_g_variant_new_ay((const guint8 *) ssid, strlen(ssid)));
            g_variant_builder_add(&builder, "{sv}", "ssids", g_variant_builder_end(&array_builder));
            options = g_variant_builder_end(&builder);

            nm_device_wifi_request_scan_options(NM_DEVICE_WIFI(info.device), options, nullptr, &scan_err);
            if (scan_err) {
                info.return_text = fmt::format(_("Error: Failed to scan hidden SSID: {}."),
                                               scan_err->message);
                g_clear_error(&scan_err);
                info.return_value = NMC_RESULT_ERROR_NOT_FOUND;
                goto out;
            }
        }

        ap = find_ap_on_device(info.device, bssid, ssid, false);
        if (!ap) {
            spdlog::error("AP not found, BSSID={}, SSID={}", bssid ? bssid : "", ssid ? ssid : "");
            info.return_value = NMC_RESULT_ERROR_NOT_FOUND;
            goto out;
        }
        g_ssid = nm_access_point_get_ssid(ap);
        if (g_ssid) {
            g_ssid = g_bytes_ref(g_ssid);
        }
        avail_cons = nm_device_get_available_connections(info.device);
        for (int i = 0; i < avail_cons->len; i++) {
            auto avail_con = (NMConnection *) g_ptr_array_index(avail_cons, i);
            if (nm_access_point_connection_valid(ap, NM_CONNECTION(avail_con))) {
                /* ap has been checked against bssid1, bssid2 and the ssid
                 * and now avail_con has been checked against ap.
                 */
                info.connection = static_cast<NMConnection *>(g_object_ref(avail_con));
                break;
            }
        }
        if (!info.connection) {
            /* If there are some connection data from user, create a connection and
             * fill them into proper settings. */
            info.connection = nm_simple_connection_new();

            s_wifi = (NMSettingWireless *) nm_setting_wireless_new();
            nm_connection_add_setting(info.connection, NM_SETTING(s_wifi));

            s_con = (NMSettingConnection *) nm_setting_connection_new();
            nm_connection_add_setting(info.connection, NM_SETTING(s_con));

            str_ssid = gBytesToStr(g_ssid);

            /* Set user provided connection name */
            g_object_set(s_con, NM_SETTING_CONNECTION_ID, str_ssid.c_str(), NULL);

            /* 'bssid' parameter is used to restrict the connection only to the BSSID */
            if(bssid) {
                g_object_set(s_wifi, NM_SETTING_WIRELESS_BSSID, bssid, NULL);
            }

            /* 'hidden' parameter is used to indicate that SSID is not broadcasted */
            if (hidden) {
                g_object_set(s_wifi,
                             NM_SETTING_WIRELESS_SSID,
                             str_ssid.c_str(),
                             NM_SETTING_WIRELESS_HIDDEN,
                             TRUE,
                             NULL);
            }
        }
        /* handle password */
        ap_flags = nm_access_point_get_flags(ap);
        ap_wpa_flags = nm_access_point_get_wpa_flags(ap);
        ap_rsn_flags = nm_access_point_get_rsn_flags(ap);
        /* Set password for WEP or WPA-PSK. */
        if (   (ap_flags & NM_802_11_AP_FLAGS_PRIVACY)
               || ap_wpa_flags != NM_802_11_AP_SEC_NONE
               || ap_rsn_flags != NM_802_11_AP_SEC_NONE) {
            NMSettingWirelessSecurity *s_wsec = nullptr;

            if (!password.empty()) {
                s_wsec = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
                nm_connection_add_setting(info.connection, NM_SETTING(s_wsec));
                if (ap_wpa_flags == NM_802_11_AP_SEC_NONE && ap_rsn_flags == NM_802_11_AP_SEC_NONE) {
                    /* WEP */
                    nm_setting_wireless_security_set_wep_key(s_wsec, 0, password.c_str());
                    g_object_set(G_OBJECT(s_wsec),
                                 NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                                 NM_WEP_KEY_TYPE_PASSPHRASE,
                                 NULL);
                } else if (   (ap_wpa_flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                              || (ap_rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)) {
                    /* WPA PSK */
                    g_object_set(s_wsec, NM_SETTING_WIRELESS_SECURITY_PSK, password.c_str(), NULL);
                    g_object_set(s_wsec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, "wpa-psk", NULL);
                }
            }
        }
        info.loop = g_main_loop_new(nullptr, FALSE);
        info.hotspot = FALSE;
        info.create = !NM_IS_REMOTE_CONNECTION(info.connection);
        info.specific_object = nm_object_get_path(NM_OBJECT(ap));
        save_and_activate_connection(&info);
        g_main_loop_run(info.loop);
        g_main_loop_unref(info.loop);
        out:
        g_object_unref(client);
        g_clear_error(&error);
        g_bytes_unref(g_ssid);
        g_bytes_unref(bssid_bytes);
        if (info.return_value != NMC_RESULT_SUCCESS) {
            spdlog::warn(info.return_text);
        }
        return info.return_value;
    }

    static NMConnection *
    find_hotspot_conn(NMDevice *device,
                      const GPtrArray *connections,
                      const char *con_name,
                      GBytes *ssid_bytes,
                      const char *wifi_mode,
                      const char *band,
                      gint64 channel_int) {
        NMConnection *connection;
        NMSettingWireless *s_wifi;
        int i;

        for (i = 0; i < connections->len; i++) {
            connection = NM_CONNECTION(connections->pdata[i]);

            s_wifi = nm_connection_get_setting_wireless(connection);
            if (!s_wifi)
                continue;

            if (channel_int != -1 && nm_setting_wireless_get_channel(s_wifi) != channel_int)
                continue;

            if (g_strcmp0(nm_setting_wireless_get_mode(s_wifi), wifi_mode) != 0)
                continue;

            if (band && g_strcmp0(nm_setting_wireless_get_band(s_wifi), band) != 0)
                continue;

            if (ssid_bytes && !g_bytes_equal(nm_setting_wireless_get_ssid(s_wifi), ssid_bytes))
                continue;

            if (!nm_device_connection_compatible(device, connection, nullptr))
                continue;

            return static_cast<NMConnection *>(g_object_ref(connection));
        }

        return nullptr;
    }

    static GBytes *
    generate_ssid_for_hotspot(void) {
        GBytes *ssid_bytes;
        char *ssid = NULL;

        ssid = g_strdup_printf("Hotspot-%s", g_get_host_name());
        if (strlen(ssid) > 32)
            ssid[32] = '\0';
        ssid_bytes = g_bytes_new(ssid, strlen(ssid));
        g_free(ssid);

        return ssid_bytes;
    }

    static NMConnection *
    create_hotspot_conn(const GPtrArray *connections,
                        const char *con_name,
                        GBytes *ssid_bytes,
                        const char *wifi_mode,
                        const char *band,
                        gint64 channel_int) {
        char *default_name = NULL;
        NMConnection *connection;
        NMSettingConnection *s_con;
        NMSettingWireless *s_wifi;
        NMSettingWirelessSecurity *s_wsec;
        NMSettingIPConfig *s_ip4, *s_ip6;
        NMSettingProxy *s_proxy;

        nm_assert(channel_int == -1 || band);

        connection = nm_simple_connection_new();
        s_con = (NMSettingConnection *) nm_setting_connection_new();
        nm_connection_add_setting(connection, NM_SETTING(s_con));
        if (!con_name)
            con_name = default_name = nmc_unique_connection_name(connections, "Hotspot");
        g_object_set(s_con,
                     NM_SETTING_CONNECTION_ID,
                     con_name,
                     NM_SETTING_CONNECTION_AUTOCONNECT,
                     FALSE,
                     NULL);
        g_free(default_name);

        s_wifi = (NMSettingWireless *) nm_setting_wireless_new();
        nm_connection_add_setting(connection, NM_SETTING(s_wifi));

        g_object_set(s_wifi,
                     NM_SETTING_WIRELESS_MODE,
                     wifi_mode,
                     NM_SETTING_WIRELESS_SSID,
                     ssid_bytes,
                     NULL);

        if (channel_int != -1) {
            g_object_set(s_wifi,
                         NM_SETTING_WIRELESS_CHANNEL,
                         (guint32) channel_int,
                         NM_SETTING_WIRELESS_BAND,
                         band,
                         NULL);
        } else if (band) {
            g_object_set(s_wifi, NM_SETTING_WIRELESS_BAND, band, NULL);
        }

        s_wsec = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
        nm_connection_add_setting(connection, NM_SETTING(s_wsec));

        s_ip4 = (NMSettingIPConfig *) nm_setting_ip4_config_new();
        nm_connection_add_setting(connection, NM_SETTING(s_ip4));
        g_object_set(s_ip4, NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_SHARED, NULL);

        s_ip6 = (NMSettingIPConfig *) nm_setting_ip6_config_new();
        nm_connection_add_setting(connection, NM_SETTING(s_ip6));
        g_object_set(s_ip6, NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP6_CONFIG_METHOD_IGNORE, NULL);

        s_proxy = (NMSettingProxy *) nm_setting_proxy_new();
        nm_connection_add_setting(connection, NM_SETTING(s_proxy));
        g_object_set(s_proxy, NM_SETTING_PROXY_METHOD, (int) NM_SETTING_PROXY_METHOD_NONE, NULL);

        return connection;
    }

#define WPA_PASSKEY_SIZE 12

    static void
    generate_wpa_key(char *key, size_t len) {
        guint i;

        g_return_if_fail (key);
        g_return_if_fail (len > WPA_PASSKEY_SIZE);

        /* generate a 8-chars ASCII WPA key */
        for (i = 0; i < WPA_PASSKEY_SIZE; i++) {
            int c;
            c = g_random_int_range (33, 126);
            /* too many non alphanumeric characters are hard to remember for humans */
            while (!g_ascii_isalnum (c))
                c = g_random_int_range (33, 126);

            key[i] = (char) c;
        }
        key[WPA_PASSKEY_SIZE] = '\0';
    }

    static void
    generate_wep_key(char *key, size_t len) {
        int i;
        const char *hexdigits = "0123456789abcdef";

        g_return_if_fail (key);
        g_return_if_fail (len > 10);

        /* generate a 10-digit hex WEP key */
        for (i = 0; i < 10; i++) {
            int digit;
            digit = g_random_int_range (0, 16);
            key[i] = hexdigits[digit];
        }
        key[10] = '\0';
    }

    static gboolean
    set_wireless_security_for_hotspot(NMSettingWirelessSecurity *s_wsec,
                                      const char *wifi_mode,
                                      NMDeviceWifiCapabilities caps,
                                      const std::string &password,
                                      GError **error) {
        char generated_key[20];
        const char *key;
        const char *key_mgmt;

        if (g_strcmp0(wifi_mode, NM_SETTING_WIRELESS_MODE_AP) == 0) {
            if (caps & NM_WIFI_DEVICE_CAP_RSN) {
                nm_setting_wireless_security_add_proto(s_wsec, "rsn");
                nm_setting_wireless_security_add_pairwise(s_wsec, "ccmp");
                nm_setting_wireless_security_add_group(s_wsec, "ccmp");
                key_mgmt = "wpa-psk";
            } else if (caps & NM_WIFI_DEVICE_CAP_WPA) {
                nm_setting_wireless_security_add_proto(s_wsec, "wpa");
                nm_setting_wireless_security_add_pairwise(s_wsec, "tkip");
                nm_setting_wireless_security_add_group(s_wsec, "tkip");
                key_mgmt = "wpa-psk";
            } else
                key_mgmt = "none";
        } else
            key_mgmt = "none";

        if (g_strcmp0(key_mgmt, "wpa-psk") == 0) {
            /* use WPA */
            if (!password.empty()) {
                if (!nm_utils_wpa_psk_valid(password.c_str())) {
                    g_set_error(error, NMCLI_ERROR, 0, _("'%s' is not valid WPA PSK"), password.c_str());
                    return FALSE;
                }
                key = password.c_str();
            } else {
                generate_wpa_key(generated_key, sizeof(generated_key));
                key = generated_key;
            }
            g_object_set(s_wsec,
                         NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                         key_mgmt,
                         NM_SETTING_WIRELESS_SECURITY_PSK,
                         key,
                         NULL);
        } else {
            /* use WEP */
            if (!password.empty()) {
                if (!nm_utils_wep_key_valid(password.c_str(), NM_WEP_KEY_TYPE_KEY)) {
                    g_set_error(error,
                                NMCLI_ERROR,
                                0,
                                _("'%s' is not valid WEP key (it should be 5 or 13 ASCII chars)"),
                                password.c_str());
                    return FALSE;
                }
                key = password.c_str();
            } else {
                generate_wep_key(generated_key, sizeof(generated_key));
                key = generated_key;
            }
            g_object_set(s_wsec,
                         NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                         key_mgmt,
                         NM_SETTING_WIRELESS_SECURITY_WEP_KEY0,
                         key,
                         NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                         NM_WEP_KEY_TYPE_KEY,
                         NULL);
        }
        return TRUE;
    }


    NMCResultCode do_device_wifi_hotspot(const std::string &interface_name, const std::string &ssid,
                                         const std::string &password) {
        GError *error = nullptr;
        const GPtrArray *connections;
        NMConnection *connection = nullptr;
        NMDevice *device;
        GBytes *ssid_bytes = nullptr;
        NMSettingWirelessSecurity *s_wsec = nullptr;
        if (!ssid.empty()) {
            ssid_bytes = g_bytes_new(ssid.c_str(), ssid.size());
        }
        NMDeviceWifiCapabilities caps;
        const char *wifi_mode;
        WifiConnectData data{
                .return_value = NMC_RESULT_SUCCESS,
        };
        data.client = nm_client_new(nullptr, &error);
        if (error) {
            spdlog::error("Error creating NMClient: {}", error->message);
            data.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }
        /* Find Wi-Fi device. When no ifname is provided, the first Wi-Fi is used. */
        device = nm_client_get_device_by_iface(data.client, interface_name.c_str());
        if (!device) {
            spdlog::error("Error: Device '%s' is not a Wi-Fi device.", interface_name);
            goto out;
        }

        /* Check device supported mode */
        caps = nm_device_wifi_get_capabilities(NM_DEVICE_WIFI(device));
        if (caps & NM_WIFI_DEVICE_CAP_AP)
            wifi_mode = NM_SETTING_WIRELESS_MODE_AP;
        else if (caps & NM_WIFI_DEVICE_CAP_ADHOC)
            wifi_mode = NM_SETTING_WIRELESS_MODE_ADHOC;
        else {
            spdlog::error("Error: Device '%s' does not support AP or Ad-Hoc mode.", interface_name);
            data.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }

        connections = nm_client_get_connections(data.client);
        connection = find_hotspot_conn(device, connections, nullptr, ssid_bytes, wifi_mode, nullptr, -1);
        if (!connection) {
            /* Create a connection with appropriate parameters */
            if (!ssid_bytes)
                ssid_bytes = generate_ssid_for_hotspot();
            connection = create_hotspot_conn(connections, nullptr, ssid_bytes, wifi_mode, nullptr, -1);
        }

        if (!password.empty() || !NM_IS_REMOTE_CONNECTION(connection)) {
            s_wsec = nm_connection_get_setting_wireless_security(connection);
            if (!s_wsec) {
                spdlog::error("Error: Can't get wireless security from connection.");
                data.return_value = NMC_RESULT_ERROR_UNKNOWN;
                goto out;
            }
            if (!set_wireless_security_for_hotspot(s_wsec,
                                                   wifi_mode,
                                                   caps,
                                                   password,
                                                   &error)) {
                data.return_text = fmt::format(_("Error: Invalid 'password': {}."), error->message);
                g_clear_error(&error);
                data.return_value = NMC_RESULT_ERROR_UNKNOWN;
                goto out;
            }
        }
        data.hotspot = TRUE;
        data.loop = g_main_loop_new(nullptr, FALSE);
        data.create = !NM_IS_REMOTE_CONNECTION(data.connection);
        save_and_activate_connection(&data);
        g_main_loop_run(data.loop);
        g_main_loop_unref(data.loop);
        out:
        g_object_unref(connection);
        g_object_unref(data.client);
        g_bytes_unref(ssid_bytes);
        g_clear_error(&error);
        if (data.return_value != NMC_RESULT_SUCCESS) {
            spdlog::warn(data.return_text);
        }
        return data.return_value;
    }

    struct DoConnectionData {
        NMClient *client;
        NMConnection *connection{nullptr};
        GMainLoop *loop{nullptr};
        NMDevice *device{nullptr};
        std::string return_text;
        NMCResultCode return_value{NMC_RESULT_SUCCESS};
        NMActiveConnection *active{nullptr};

        void quite();
    };

    NMConnection *find_connection_by_id(NMDevice *device, const char *id) {
        auto connections = nm_device_get_available_connections(device);
        for (guint i = 0; i < connections->len; i++) {
            auto conn = static_cast<NMConnection *>(g_ptr_array_index(connections, i));
            auto conn_id = nm_connection_get_id(conn);
            if (g_strcmp0(conn_id, id) == 0) {
                return conn;
            }
        }
        return nullptr;
    }

    static void
    active_connection_hint(std::string &return_text, NMActiveConnection *active, NMDevice *device) {
        NMRemoteConnection *connection;
        nm_auto_free_gstring GString *hint = nullptr;
        const GPtrArray *devices;
        guint i;

        if (!active)
            return;

        connection = nm_active_connection_get_connection(active);
        g_return_if_fail(connection);

        hint = g_string_new("journalctl -xe ");
        g_string_append_printf(hint,
                               "NM_CONNECTION=%s",
                               nm_connection_get_uuid(NM_CONNECTION(connection)));

        if (device)
            g_string_append_printf(hint, " + NM_DEVICE=%s", nm_device_get_iface(device));
        else {
            devices = nm_active_connection_get_devices(active);
            for (i = 0; i < devices->len; i++) {
                g_string_append_printf(hint,
                                       " + NM_DEVICE=%s",
                                       nm_device_get_iface(NM_DEVICE(g_ptr_array_index(devices, i))));
            }
        }
        return_text.append("\n").append(fmt::format(_("Hint: use '{}' to get more details."), hint->str));
    }

    static void
    check_activated(DoConnectionData *info) {
        NMActiveConnectionState ac_state;
        const char *reason = NULL;

        ac_state = nmc_activation_get_effective_state(info->active, info->device, &reason);
        switch (ac_state) {
            case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
                if (reason) {
                    spdlog::info(_("Connection successfully activated ({}) (D-Bus active path: {})\n"),
                                 reason,
                                 nm_object_get_path(NM_OBJECT(info->active)));
                } else {
                    spdlog::info(_("Connection successfully activated (D-Bus active path: {})\n"),
                                 nm_object_get_path(NM_OBJECT(info->active)));
                }
                info->quite();
                break;
            case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
                nm_assert(reason);
                info->return_text = fmt::format(_("Error: Connection activation failed: {}"), reason);
                if (nm_device_get_state_reason(info->device) == NM_DEVICE_STATE_REASON_NO_SECRETS) {
                    info->return_value = NMC_RESULT_ERROR_CON_REQUIRE_SECRET;
                } else {
                    info->return_value = NMC_RESULT_ERROR_CON_ACTIVATION;
                }
                active_connection_hint(info->return_text, info->active, info->device);
                info->quite();
                break;
            case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
                break;
            default:
                break;
        }
    }

    static void
    active_connection_state_cb(NMActiveConnection *active,
                               NMActiveConnectionState state,
                               NMActiveConnectionStateReason reason,
                               DoConnectionData *info) {
        check_activated(info);
    }

    static void
    device_state_cb(NMDevice *device, GParamSpec *pspec, DoConnectionData *info) {
        check_activated(info);
    }

    void DoConnectionData::quite() {
        g_object_unref(active);
        if (device) {
            g_signal_handlers_disconnect_by_func(device, (gpointer) (device_state_cb), this);
        }

        if (active) {
            g_signal_handlers_disconnect_by_func(active, (gpointer) (active_connection_state_cb), this);
        }
        g_main_loop_quit(loop);
    }

    static void
    activate_connection_cb(GObject *client, GAsyncResult *result, gpointer user_data) {
        auto info = (DoConnectionData *) user_data;
        GError *error = nullptr;
        info->active = nm_client_activate_connection_finish(NM_CLIENT(client), result, &error);
        if (error) {
            info->return_text = fmt::format(_("Error: Connection activation failed: {}"), error->message);
            g_error_free(error);
            info->return_value = NMC_RESULT_ERROR_CON_ACTIVATION;
            info->quite();
        } else {
            auto state = nm_active_connection_get_state(info->active);
            if (state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED) {
                spdlog::info(_("Connection successfully activated (D-Bus active path: {})"),
                             nm_object_get_path(NM_OBJECT(info->active)));
                info->quite();
            } else if (state == NM_ACTIVE_CONNECTION_STATE_DEACTIVATED) {
                auto dev_reason = nm_device_get_state_reason(info->device);
                info->return_text = fmt::format(_("Error: Connection activation failed: {}"),
                                                nmc_device_reason_to_string(dev_reason));
                if (dev_reason == NM_DEVICE_STATE_REASON_NO_SECRETS) {
                    info->return_value = NMC_RESULT_ERROR_CON_REQUIRE_SECRET;
                } else {
                    info->return_value = NMC_RESULT_ERROR_CON_ACTIVATION;
                }
                info->quite();
            } else if (state == NM_ACTIVE_CONNECTION_STATE_ACTIVATING) {
                /* Monitor the active connection and device (if available) states */
                g_signal_connect(info->active, "state-changed", G_CALLBACK(active_connection_state_cb), info);
                if (info->device)
                    g_signal_connect(info->device,
                                     "notify::" NM_DEVICE_STATE,
                                     G_CALLBACK(device_state_cb),
                                     info);
                /* Both active_connection_state_cb () and device_state_cb () will just
                 * call check_activated (info). So, just call it once directly after
                 * connecting on both the signals of the objects and skip the call to
                 * the callbacks.
                 */
                check_activated(info);
            }
        }
    }

    NMCResultCode do_connection_up(const std::string &interface_name, const std::string &id) {
        DoConnectionData data{
                .client = nm_client_new(nullptr, nullptr),
                .connection = nullptr,
                .loop = g_main_loop_new(nullptr, FALSE),
                .return_value = NMC_RESULT_SUCCESS,
        };
        const char *spec_object;
        if (!data.client) {
            data.return_text = _("Error creating NMClient");
            data.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }
        data.device = nm_client_get_device_by_iface(data.client, interface_name.c_str());
        if (!data.device) {
            data.return_text = fmt::format("Error: Device '{}' not found.", interface_name);
            data.return_value = NMC_RESULT_ERROR_NOT_FOUND;
            goto out;
        }
        data.connection = find_connection_by_id(data.device, id.c_str());
        if (!data.connection) {
            data.return_text = fmt::format("Error: Connection '{}' not found.", id);
            data.return_value = NMC_RESULT_ERROR_NOT_FOUND;
            goto out;
        }
        spec_object = nm_object_get_path(NM_OBJECT(data.connection));
        if (!spec_object) {
            data.return_text = fmt::format("Error: Can't get object path for connection '{}'.", id);
            data.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }
        nm_client_activate_connection_async(data.client, data.connection, data.device, spec_object, nullptr,
                                            activate_connection_cb, &data);
        g_main_loop_run(data.loop);
        out:
        g_object_unref(data.client);
        g_main_loop_unref(data.loop);
        if (data.return_value != NMC_RESULT_SUCCESS) {
            spdlog::warn(data.return_text);
        }
        return data.return_value;
    }

    NMCResultCode do_connection_down(const std::string &interface_name, const std::string &conn_id) {
        auto client = nm_client_new(nullptr, nullptr);
        if (!client) {
            spdlog::warn("Error creating NMClient");
            return NMC_RESULT_ERROR_UNKNOWN;
        }
        NMCResultCode ret = NMC_RESULT_SUCCESS;
        NMActiveConnection *conn;
        NMRemoteConnection *remote_conn;
        GError *error = nullptr;
        const char *cur_conn_id;
        auto device = nm_client_get_device_by_iface(client, interface_name.c_str());
        if (!device) {
            spdlog::warn("Error: Device '{}' not found.", interface_name);
            goto out;
        }
        conn = nm_device_get_active_connection(device);
        if (!conn) {
            spdlog::warn("Error: No active connection found for device '{}'.", interface_name);
            goto out;
        }
        cur_conn_id = nm_active_connection_get_id(conn);
        if (g_strcmp0(cur_conn_id, conn_id.c_str())) {
            spdlog::warn("Error: Connection '{}' does not match Id '{}'.", conn_id, cur_conn_id);
            goto out;
        }
        if (nm_client_deactivate_connection(client, conn, nullptr, &error)) {
            spdlog::info("Connection successfully deactivated.");
        } else {
            spdlog::warn("Error: Connection deactivation failed: {}", error->message);
            ret = NMC_RESULT_ERROR_CON_DEACTIVATION;
        }
        out:
        g_object_unref(client);
        g_error_free(error);
        return ret;
    }

    static void
    delete_cb(GObject *con, GAsyncResult *result, gpointer user_data) {
        auto info = (DoConnectionData *) user_data;
        GError *error = nullptr;

        if (!nm_remote_connection_delete_finish(NM_REMOTE_CONNECTION(con), result, &error)) {
            if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                return;
            info->return_text = _("Error: not all connections deleted.");
            spdlog::warn(_("Error: Connection deletion failed: %s"), error->message);
            g_error_free(error);
            info->return_value = NMC_RESULT_ERROR_CON_DEL;
        }
        info->quite();
    }

    NMCResultCode do_connection_delete(const std::string &interface_name, const std::string &conn_id) {
        DoConnectionData data{
                .client = nm_client_new(nullptr, nullptr),
                .connection = nullptr,
                .loop = g_main_loop_new(nullptr, FALSE),
        };
        if (!data.client) {
            data.return_text = _("Error creating NMClient");
            data.return_value = NMC_RESULT_ERROR_UNKNOWN;
            goto out;
        }
        data.device = nm_client_get_device_by_iface(data.client, interface_name.c_str());
        if (!data.device) {
            spdlog::warn("Error: Device '{}' not found.", interface_name);
            goto out;
        }
        data.connection = find_connection_by_id(data.device, conn_id.c_str());
        if (!data.connection) {
            spdlog::warn("Error: Connection '{}' not found.", conn_id);
            goto out;
        }
        nm_remote_connection_delete_async(NM_REMOTE_CONNECTION(data.connection), nullptr, delete_cb, &data);
        g_main_loop_run(data.loop);
        out:
        g_object_unref(data.client);
        g_main_loop_unref(data.loop);
        return data.return_value;
    }

    static std::list<ConnectionInfo> get_connections_list(const GPtrArray *conn_list) {
        std::list<ConnectionInfo> ret;
        for (guint i = 0; i < conn_list->len; i++) {
            auto conn = static_cast<NMConnection *>(g_ptr_array_index(conn_list, i));
            ConnectionInfo info;
            info.id = cStr(nm_connection_get_id(conn));
            auto setting = nm_connection_get_setting_wireless(conn);
            if (!setting) {
                continue;
            }
            info.bssid = cStr(nm_setting_wireless_get_bssid(setting));
            info.ssid = gBytesToStr(nm_setting_wireless_get_ssid(setting));
            ret.push_back(info);
        }
        return ret;
    }

    std::list<ConnectionInfo> get_connections() {
        auto client = nm_client_new(nullptr, nullptr);
        const GPtrArray *conn_list;
        if (!client) {
            spdlog::warn("Error creating NMClient");
            return {};
        }
        conn_list = nm_client_get_connections(client);
        auto ret = get_connections_list(conn_list);
        g_object_unref(client);
        return ret;
    }

    ConnectionInfo *get_active_connection(const std::string &interface_name) {
        auto client = nm_client_new(nullptr, nullptr);
        const GPtrArray *conn_list;
        if (!client) {
            spdlog::warn("Error creating NMClient");
            return nullptr;
        }
        auto device = nm_client_get_device_by_iface(client, interface_name.c_str());
        if (!device) {
            spdlog::warn("Error: Device '{}' not found.", interface_name);
            g_object_unref(client);
            return nullptr;
        }
        auto act_conn = nm_device_get_active_connection(device);
        if (!act_conn) {
            spdlog::warn("Error: No active connection found for device '{}'.", interface_name);
            g_object_unref(client);
            return nullptr;
        }
        auto r_conn = nm_active_connection_get_connection(act_conn);
        if (!r_conn) {
            spdlog::warn("Error: No connection found for active connection.");
            g_object_unref(client);
            return nullptr;
        }
        ConnectionInfo *info{nullptr};
        auto conn = NM_CONNECTION(r_conn);
        auto st = nm_connection_get_setting_wireless(conn);
        if (st) {
            info = new ConnectionInfo;
            info->id = cStr(nm_active_connection_get_id(act_conn));
            info->ssid = gBytesToStr(nm_setting_wireless_get_ssid(st));
            info->bssid = cStr(nm_setting_wireless_get_bssid(st));
        }
        g_object_unref(client);
        return info;
    }

    std::string gBytesToStr(GBytes *bytes) {
        if (!bytes) {
            return {};
        }
        gsize size{0};
        auto data = g_bytes_get_data(bytes, &size);
        return {(const char *) data, size};
    }

    std::string cStr(const char *str) {
        if (str) {
            return {str};
        }
        return {};
    }
}