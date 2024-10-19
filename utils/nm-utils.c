//
// Created by wenyiyu on 2024/10/18.
//
#include "nm-utils.h"
#include <fcntl.h>
#include <sys/auxv.h>
#include <sys/syscall.h>
#if USE_SYS_RANDOM_H
#include <sys/random.h>
#else
#include <linux/random.h>
#endif
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>

#define nm_assert(args) {}
typedef const char *const StrvArray4Type[4];

#define _NM_UTILS_STRING_TABLE_LOOKUP_DEFINE(fcn_name,                                         \
                                             value_type,                                       \
                                             value_type_result,                                \
                                             entry_cmd,                                        \
                                             unknown_val_cmd,                                  \
                                             get_operator,                                     \
                                             ...)                                              \
    value_type_result fcn_name(const char *name)                                               \
    {                                                                                          \
        static const struct {                                                                  \
            const char *name;                                                                  \
            value_type  value;                                                                 \
        } LIST[] = {__VA_ARGS__};                                                              \
                                                                                               \
                                                                                               \
        {                                                                                      \
            entry_cmd;                                                                         \
        }                                                                                      \
                                                                                               \
        if (G_LIKELY(name)) {                                                                  \
            G_STATIC_ASSERT(G_N_ELEMENTS(LIST) > 1);                                           \
            G_STATIC_ASSERT(G_N_ELEMENTS(LIST) < G_MAXINT / 2 - 10);                           \
            int imin = 0;                                                                      \
            int imax = (G_N_ELEMENTS(LIST) - 1);                                               \
            int imid = (G_N_ELEMENTS(LIST) - 1) / 2;                                           \
                                                                                               \
            for (;;) {                                                                         \
                const int cmp = strcmp(LIST[imid].name, name);                                 \
                                                                                               \
                if (G_UNLIKELY(cmp == 0))                                                      \
                    return get_operator(LIST[imid].value);                                     \
                                                                                               \
                if (cmp < 0)                                                                   \
                    imin = imid + 1;                                                           \
                else                                                                           \
                    imax = imid - 1;                                                           \
                                                                                               \
                if (G_UNLIKELY(imin > imax))                                                   \
                    break;                                                                     \
                                                                                               \
                /* integer overflow cannot happen, because LIST is shorter than G_MAXINT/2. */ \
                imid = (imin + imax) / 2;                                                      \
            }                                                                                  \
        }                                                                                      \
                                                                                               \
        {                                                                                      \
            unknown_val_cmd;                                                                   \
        }                                                                                      \
    }


#define LL(l, ...)                    \
    {                                 \
        .name  = l,                   \
        .value = {__VA_ARGS__, NULL}, \
    }

/* 5-letter language codes */
static _NM_UTILS_STRING_TABLE_LOOKUP_DEFINE(
        _iso_lang_entries_5_lookup,
        StrvArray4Type,
const char *const *,
{},
{ return NULL; },
,
LL("zh_cn", "euc-cn", "gb2312", "gb18030"), /* Simplified Chinese, PRC */
LL("zh_hk", "big5", "euc-tw", "big5-hkcs"), /* Traditional Chinese, Hong Kong */
LL("zh_mo", "big5", "euc-tw"),              /* Traditional Chinese, Macau */
LL("zh_sg", "euc-cn", "gb2312", "gb18030"), /* Simplified Chinese, Singapore */
LL("zh_tw", "big5", "euc-tw"),              /* Traditional Chinese, Taiwan */
);

/* 2-letter language codes; we don't care about the other 3 in this table */
static _NM_UTILS_STRING_TABLE_LOOKUP_DEFINE(
        _iso_lang_entries_2_lookup,
        StrvArray4Type,
        const char *const *,
        { nm_assert(name); },
        { return NULL; },
,
        LL("ar", "iso-8859-6", "windows-1256"),           /* Arabic */
        LL("be", "koi8-r", "windows-1251", "iso-8859-5"), /* Cyrillic, Belorussian */
        LL("bg", "windows-1251", "koi8-r", "iso-8859-5"), /* Cyrillic, Bulgarian */
        LL("cs", "iso-8859-2", "windows-1250"),           /* Central European, Czech */
        LL("el", "iso-8859-7", "windows-1253"),           /* Greek */
        LL("et", "iso-8859-4", "windows-1257"),           /* Baltic, Estonian */
        LL("he", "iso-8859-8", "windows-1255"),           /* Hebrew */
        LL("hr", "iso-8859-2", "windows-1250"),           /* Central European, Croatian */
        LL("hu", "iso-8859-2", "windows-1250"),           /* Central European, Hungarian */
        LL("iw", "iso-8859-8", "windows-1255"),           /* Hebrew */
        LL("ja", "euc-jp", "shift_jis", "iso-2022-jp"),   /* Japanese */
        LL("ko", "euc-kr", "iso-2022-kr", "johab"),       /* Korean */
        LL("lt", "iso-8859-4", "windows-1257"),           /* Baltic, Lithuanian */
        LL("lv", "iso-8859-4", "windows-1257"),           /* Baltic, Latvian */
        LL("mk", "koi8-r", "windows-1251", "iso-8859-5"), /* Cyrillic, Macedonian */
        LL("pl", "iso-8859-2", "windows-1250"),           /* Central European, Polish */
        LL("ro", "iso-8859-2", "windows-1250"),           /* Central European, Romanian */
        LL("ru", "koi8-r", "windows-1251", "iso-8859-5"), /* Cyrillic, Russian */
        LL("sh", "iso-8859-2", "windows-1250"),           /* Central European, Serbo-Croatian */
        LL("sk", "iso-8859-2", "windows-1250"),           /* Central European, Slovakian */
        LL("sl", "iso-8859-2", "windows-1250"),           /* Central European, Slovenian */
        LL("sr", "koi8-r", "windows-1251", "iso-8859-5"), /* Cyrillic, Serbian */
        LL("th", "iso-8859-11", "windows-874"),           /* Thai */
        LL("tr", "iso-8859-9", "windows-1254"),           /* Turkish */
        LL("uk", "koi8-u", "koi8-r", "windows-1251"),     /* Cyrillic, Ukrainian */
);

static const char *const *
_system_encodings_for_lang(const char *lang)
{
    char               tmp_lang[3];
    const char *const *e;

    if (lang[0] == '\0' || lang[1] == '\0') {
        /* need at least two characters. */
        return NULL;
    }

    if (lang[2] != '\0') {
        if (lang[3] != '\0' && lang[4] != '\0' && lang[5] == '\0') {
            /* lang is 5 characters long. Try it. */
            if ((e = _iso_lang_entries_5_lookup(lang)))
                return e;
        }

        /* extract the first 2 characters and ignore the rest. */
        tmp_lang[0] = lang[0];
        tmp_lang[1] = lang[1];
        tmp_lang[2] = '\0';
        lang        = tmp_lang;
    }

    if ((e = _iso_lang_entries_2_lookup(lang)))
        return e;

    return NULL;
}

static const char *const *
_system_encodings_get_default(void)
{
    static gsize       init_once = 0;
    static const char *default_encodings[4];

    if (g_once_init_enter(&init_once)) {
        const char *e_default = NULL;
        int         i;

        g_get_charset(&e_default);

        i = 0;
        if (e_default)
            default_encodings[i++] = e_default;
        if (!nm_streq0(e_default, "iso-8859-1"))
            default_encodings[i++] = "iso-8859-1";
        if (!nm_streq0(e_default, "windows-1251"))
            default_encodings[i++] = "windows-1251";
        default_encodings[i++] = NULL;
        nm_assert(i <= G_N_ELEMENTS(default_encodings));

        g_once_init_leave(&init_once, 1);
    }

    return default_encodings;
}

static const char *const *
_system_encodings_get(void)
{
    static const char *const *cached = NULL;
    const char *const        *e;

    again:
    if (!(e = g_atomic_pointer_get(&cached))) {
        const char *lang;

        /* Use environment variables as encoding hint */
        lang = getenv("LC_ALL") ?: getenv("LC_CTYPE") ?: getenv("LANG");

        if (lang) {
            char *lang_down = NULL;
            char         *dot;

            lang_down = g_ascii_strdown(lang, -1);
            if ((dot = strchr(lang_down, '.')))
                *dot = '\0';
            e = _system_encodings_for_lang(lang_down);
            g_free(lang_down);
        }

        if (!e)
            e = _system_encodings_get_default();

        /* in any case, @e is now a static buffer, that we may cache. */
        nm_assert(e);

        if (!g_atomic_pointer_compare_and_exchange(&cached, NULL, e))
            goto again;
    }

    return e;
}


/**
 * nm_utils_ssid_to_utf8:
 * @ssid: (array length=len): pointer to a buffer containing the SSID data
 * @len: length of the SSID data in @ssid
 *
 * Wi-Fi SSIDs are byte arrays, they are _not_ strings.  Thus, an SSID may
 * contain embedded NULLs and other unprintable characters.  Often it is
 * useful to print the SSID out for debugging purposes, but that should be the
 * _only_ use of this function.  Do not use this function for any persistent
 * storage of the SSID, since the printable SSID returned from this function
 * cannot be converted back into the real SSID of the access point.
 *
 * This function does almost everything humanly possible to convert the input
 * into a printable UTF-8 string, using roughly the following procedure:
 *
 * 1) if the input data is already UTF-8 safe, no conversion is performed
 * 2) attempts to get the current system language from the LANG environment
 *    variable, and depending on the language, uses a table of alternative
 *    encodings to try.  For example, if LANG=hu_HU, the table may first try
 *    the ISO-8859-2 encoding, and if that fails, try the Windows-1250 encoding.
 *    If all fallback encodings fail, replaces non-UTF-8 characters with '?'.
 * 3) If the system language was unable to be determined, falls back to the
 *    ISO-8859-1 encoding, then to the Windows-1251 encoding.
 * 4) If step 3 fails, replaces non-UTF-8 characters with '?'.
 *
 * Again, this function should be used for debugging and display purposes
 * _only_.
 *
 * Returns: (transfer full): an allocated string containing a UTF-8
 * representation of the SSID, which must be freed by the caller using g_free().
 * Returns %NULL on errors.
 **/
char *
nm_utils_ssid_to_utf8(const guint8 *ssid, gsize len)
{
    const char *const *encodings;
    const char *const *e;
    char              *converted = NULL;

    g_return_val_if_fail(ssid != NULL, NULL);

    if (g_utf8_validate((const char *) ssid, len, NULL))
        return g_strndup((const char *) ssid, len);

    encodings = _system_encodings_get();

    for (e = encodings; *e; e++) {
        converted = g_convert((const char *) ssid, len, "UTF-8", *e, NULL, NULL, NULL);
        if (converted)
            break;
    }

    if (!converted) {
        converted = g_convert_with_fallback((const char *) ssid,
                                            len,
                                            "UTF-8",
                                            encodings[0],
                                            "?",
                                            NULL,
                                            NULL,
                                            NULL);
    }

    if (!converted) {
        /* If there is still no converted string, the SSID probably
         * contains characters not valid in the current locale. Convert
         * the string to ASCII instead.
         */

        /* Use the printable range of 0x20-0x7E */
        char *valid_chars = " !\"#$%&'()*+,-./0123456789:;<=>?@"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                            "abcdefghijklmnopqrstuvwxyz{|}~";

        converted = g_strndup((const char *) ssid, len);
        g_strcanon(converted, valid_chars, '?');
    }

    return converted;
}

const char *nmc_device_reason_to_string(NMDeviceStateReason reason) {
    switch (reason) {
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_NONE, N_("No reason given"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_UNKNOWN, N_("Unknown error"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_NOW_MANAGED, N_("Device is now managed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_NOW_UNMANAGED, N_("Device is now unmanaged"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_CONFIG_FAILED,
                             N_("The device could not be readied for configuration"))
        NM_UTILS_LOOKUP_ITEM(
                NM_DEVICE_STATE_REASON_IP_CONFIG_UNAVAILABLE,
                N_("IP configuration could not be reserved (no available address, timeout, etc.)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_IP_CONFIG_EXPIRED,
                             N_("The IP configuration is no longer valid"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_NO_SECRETS,
                             N_("Secrets were required, but not provided"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SUPPLICANT_DISCONNECT,
                             N_("802.1X supplicant disconnected"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SUPPLICANT_CONFIG_FAILED,
                             N_("802.1X supplicant configuration failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SUPPLICANT_FAILED, N_("802.1X supplicant failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SUPPLICANT_TIMEOUT,
                             N_("802.1X supplicant took too long to authenticate"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PPP_START_FAILED,
                             N_("PPP service failed to start"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PPP_DISCONNECT, N_("PPP service disconnected"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PPP_FAILED, N_("PPP failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_DHCP_START_FAILED,
                             N_("DHCP client failed to start"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_DHCP_ERROR, N_("DHCP client error"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_DHCP_FAILED, N_("DHCP client failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SHARED_START_FAILED,
                             N_("Shared connection service failed to start"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SHARED_FAILED,
                             N_("Shared connection service failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_AUTOIP_START_FAILED,
                             N_("AutoIP service failed to start"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_AUTOIP_ERROR, N_("AutoIP service error"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_AUTOIP_FAILED, N_("AutoIP service failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_BUSY, N_("The line is busy"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_NO_DIAL_TONE, N_("No dial tone"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_NO_CARRIER,
                             N_("No carrier could be established"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_DIAL_TIMEOUT,
                             N_("The dialing request timed out"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_DIAL_FAILED,
                             N_("The dialing attempt failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_INIT_FAILED,
                             N_("Modem initialization failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_APN_FAILED,
                             N_("Failed to select the specified APN"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_REGISTRATION_NOT_SEARCHING,
                             N_("Not searching for networks"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_REGISTRATION_DENIED,
                             N_("Network registration denied"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_REGISTRATION_TIMEOUT,
                             N_("Network registration timed out"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_REGISTRATION_FAILED,
                             N_("Failed to register with the requested network"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_PIN_CHECK_FAILED, N_("PIN check failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_FIRMWARE_MISSING,
                             N_("Necessary firmware for the device may be missing"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_REMOVED, N_("The device was removed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SLEEPING, N_("NetworkManager went to sleep"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_CONNECTION_REMOVED,
                             N_("The device's active connection disappeared"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_USER_REQUESTED,
                             N_("Device disconnected by user or client"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_CARRIER, N_("Carrier/link changed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_CONNECTION_ASSUMED,
                             N_("The device's existing connection was assumed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE,
                             N_("The supplicant is now available"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_NOT_FOUND,
                             N_("The modem could not be found"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_BT_FAILED,
                             N_("The Bluetooth connection failed or timed out"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_SIM_NOT_INSERTED,
                             N_("GSM Modem's SIM card not inserted"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_SIM_PIN_REQUIRED,
                             N_("GSM Modem's SIM PIN required"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_SIM_PUK_REQUIRED,
                             N_("GSM Modem's SIM PUK required"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_GSM_SIM_WRONG, N_("GSM Modem's SIM wrong"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_INFINIBAND_MODE,
                             N_("InfiniBand device does not support connected mode"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_DEPENDENCY_FAILED,
                             N_("A dependency of the connection failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_BR2684_FAILED,
                             N_("A problem with the RFC 2684 Ethernet over ADSL bridge"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_MANAGER_UNAVAILABLE,
                             N_("ModemManager is unavailable"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SSID_NOT_FOUND,
                             N_("The Wi-Fi network could not be found"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SECONDARY_CONNECTION_FAILED,
                             N_("A secondary connection of the base connection failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_DCB_FCOE_FAILED, N_("DCB or FCoE setup failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_TEAMD_CONTROL_FAILED, N_("teamd control failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_FAILED,
                             N_("Modem failed or no longer available"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_MODEM_AVAILABLE,
                             N_("Modem now ready and available"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SIM_PIN_INCORRECT, N_("SIM PIN was incorrect"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_NEW_ACTIVATION,
                             N_("New connection activation was enqueued"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PARENT_CHANGED, N_("The device's parent changed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PARENT_MANAGED_CHANGED,
                             N_("The device parent's management changed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_OVSDB_FAILED,
                             N_("Open vSwitch database connection failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_IP_ADDRESS_DUPLICATE,
                             N_("A duplicate IP address was detected"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_IP_METHOD_UNSUPPORTED,
                             N_("The selected IP method is not supported"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_SRIOV_CONFIGURATION_FAILED,
                             N_("Failed to configure SR-IOV parameters"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_REASON_PEER_NOT_FOUND,
                             N_("The Wi-Fi P2P peer could not be found"))
    }
    /* TRANSLATORS: Unknown reason for a device state change (NMDeviceStateReason) */
    NM_UTILS_LOOKUP_DEFAULT(N_("Unknown"));
}


const char *nmc_device_state_to_string(NMDeviceState state) {
    switch (state) {
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_UNMANAGED, N_("unmanaged"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_UNAVAILABLE, N_("unavailable"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_DISCONNECTED, N_("disconnected"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_PREPARE, N_("connecting (prepare)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_CONFIG, N_("connecting (configuring)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_NEED_AUTH, N_("connecting (need authentication)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_IP_CONFIG, N_("connecting (getting IP configuration)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_IP_CHECK, N_("connecting (checking IP connectivity)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_SECONDARIES,
                             N_("connecting (starting secondary connections)"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_ACTIVATED, N_("connected"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_DEACTIVATING, N_("deactivating"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_FAILED, N_("connection failed"))
        NM_UTILS_LOOKUP_ITEM(NM_DEVICE_STATE_UNKNOWN, N_("unknown"));
    }
    NM_UTILS_LOOKUP_DEFAULT(N_("unknown"));
}

char *
nmc_unique_connection_name(const GPtrArray *connections, const char *try_name)
{
    NMConnection *connection;
    const char   *name;
    char         *new_name;
    unsigned      num = 1;
    int           i   = 0;

    new_name = g_strdup(try_name);
    while (i < connections->len) {
        connection = NM_CONNECTION(connections->pdata[i]);

        name = nm_connection_get_id(connection);
        if (g_strcmp0(new_name, name) == 0) {
            g_free(new_name);
            new_name = g_strdup_printf("%s-%d", try_name, num++);
            i        = 0;
        } else
            i++;
    }
    return new_name;
}

NM_CACHED_QUARK_FCN("nmcli-error-quark", nmcli_error_quark);

#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK 0x01
#endif

#if !HAVE_GETRANDOM
static ssize_t
getrandom(void *buf, size_t buflen, unsigned flags)
{
#if defined(SYS_getrandom)
    return syscall(SYS_getrandom, buf, buflen, flags);
#else
    errno = ENOSYS;
    return -1;
#endif
}
#endif

static ssize_t
_getrandom(void *buf, size_t buflen, unsigned flags)
{
    static int have_getrandom = TRUE;
    ssize_t    l;
    int        errsv;

    nm_assert(buflen > 0);

    /* This calls getrandom() and either returns the positive
     * success or an negative errno. ENOSYS means getrandom()
     * call is not supported. That result is cached and we don't retry. */

    if (!have_getrandom)
        return -ENOSYS;

    l = getrandom(buf, buflen, flags);
    if (l > 0)
        return l;
    if (l == 0)
        return -EIO;
    errsv = errno;
    if (errsv == ENOSYS)
        have_getrandom = FALSE;
    return -errsv;
}

/* Note: @value is only evaluated when *out_val is present.
 * Thus,
 *    NM_SET_OUT (out_str, g_strdup ("hallo"));
 * does the right thing.
 */
#define NM_SET_OUT(out_val, value)                \
    ({                                            \
        typeof(*(out_val)) *_out_val = (out_val); \
                                                  \
        if (_out_val) {                           \
            *_out_val = (value);                  \
        }                                         \
                                                  \
        (!!_out_val);                             \
    })

inline void
nm_close(int fd)
{
    NM_AUTO_PROTECT_ERRNO(errsv);

    nm_close_with_error(fd);
}

inline void
_nm_auto_close(int *pfd)
{
    if (*pfd >= 0)
        nm_close(*pfd);
}

/* taken from systemd's fd_wait_for_event(). Note that the timeout
 * is here in nano-seconds, not micro-seconds. */
int
nm_utils_fd_wait_for_event(int fd, int event, gint64 timeout_nsec)
{
    struct pollfd pollfd = {
            .fd     = fd,
            .events = event,
    };
    struct timespec ts, *pts;
    int             r;

    nm_assert(fd >= 0);

    if (timeout_nsec < 0)
        pts = NULL;
    else {
        ts.tv_sec  = (time_t) (timeout_nsec / NM_UTILS_NSEC_PER_SEC);
        ts.tv_nsec = (long int) (timeout_nsec % NM_UTILS_NSEC_PER_SEC);
        pts        = &ts;
    }

    r = ppoll(&pollfd, 1, pts, NULL);
    if (r < 0)
        return -NM_ERRNO_NATIVE(errno);
    if (r == 0)
        return 0;

    nm_assert(r == 1);
    nm_assert(pollfd.revents > 0);

    if (pollfd.revents & POLLNVAL)
        return -EBADF;

    return pollfd.revents;
}

static int
_random_check_entropy(gboolean block)
{
    static gboolean   seen_high_quality = FALSE;
    nm_auto_close int fd                = -1;
    int               r;

    /* We come here because getrandom() gave ENOSYS. We will fallback to /dev/urandom,
     * but the caller wants to know whether we have high quality numbers. Poll
     * /dev/random to find out. */

    if (seen_high_quality) {
        /* We cache the positive result. Once kernel has entropy, we will get
         * good random numbers. */
        return 1;
    }

    fd = open("/dev/random", O_RDONLY | O_CLOEXEC | O_NOCTTY);
    if (fd < 0)
        return -errno;

    r = nm_utils_fd_wait_for_event(fd, POLLIN, block ? -1 : 0);

    if (r <= 0) {
        nm_assert(r < 0 || !block);
        return r;
    }

    nm_assert(r == 1);
    seen_high_quality = TRUE;
    return 1;
}

static ssize_t
_getrandom_insecure(void *buf, size_t buflen)
{
    static int have_grnd_insecure = TRUE;
    ssize_t    l;

    /* GRND_INSECURE was added recently. We catch EINVAL
     * if kernel does not support the flag (and cache it). */

    if (!have_grnd_insecure)
        return -EINVAL;

    l = _getrandom(buf, buflen, GRND_INSECURE);

    if (l == -EINVAL)
        have_grnd_insecure = FALSE;

    return l;
}

/* taken from systemd's loop_read() */
ssize_t
nm_utils_fd_read_loop(int fd, void *buf, size_t nbytes, bool do_poll)
{
    uint8_t *p = buf;
    ssize_t  n = 0;

    g_return_val_if_fail(fd >= 0, -EINVAL);
    g_return_val_if_fail(buf, -EINVAL);

    /* If called with nbytes == 0, let's call read() at least
     * once, to validate the operation */

    if (nbytes > (size_t) SSIZE_MAX)
        return -EINVAL;

    do {
        ssize_t k;

        k = read(fd, p, nbytes);
        if (k < 0) {
            int errsv = errno;

            if (errsv == EINTR)
                continue;

            if (errsv == EAGAIN && do_poll) {
                /* We knowingly ignore any return value here,
                 * and expect that any error/EOF is reported
                 * via read() */

                (void) nm_utils_fd_wait_for_event(fd, POLLIN, -1);
                continue;
            }

            return n > 0 ? n : -NM_ERRNO_NATIVE(errsv);
        }

        if (k == 0)
            return n;

        g_assert((size_t) k <= nbytes);

        p += k;
        nbytes -= k;
        n += k;
    } while (nbytes > 0);

    return n;
}


/* taken from systemd's loop_read_exact() */
int
nm_utils_fd_read_loop_exact(int fd, void *buf, size_t nbytes, bool do_poll)
{
    ssize_t n;

    n = nm_utils_fd_read_loop(fd, buf, nbytes, do_poll);
    if (n < 0)
        return (int) n;
    if ((size_t) n != nbytes)
        return -EIO;

    return 0;
}

typedef struct _nm_packed {
    guint64 counter;
    union {
        guint8 full[NM_UTILS_CHECKSUM_LENGTH_SHA256];
        struct {
            guint8 half_1[NM_UTILS_CHECKSUM_LENGTH_SHA256 / 2];
            guint8 half_2[NM_UTILS_CHECKSUM_LENGTH_SHA256 / 2];
        };
    } sha_digest;
    union {
        guint8  u8[NM_UTILS_CHECKSUM_LENGTH_SHA256 / 2];
        guint32 u32[((NM_UTILS_CHECKSUM_LENGTH_SHA256 / 2) + 3) / 4];
    } rand_vals;
    guint8 rand_vals_getrandom[16];
    gint64 rand_vals_timestamp;
} BadRandState;

typedef struct _nm_bad_rand_seed_packed {
    uintptr_t heap_ptr;
    uintptr_t stack_ptr;
    gint64    now_bootime;
    gint64    now_real;
    pid_t     pid;
    pid_t     ppid;
    pid_t     tid;
    guint32   grand[16];
    guint8    auxval[16];
    guint8    getrandom_buf[20];
} BadRandSeed;

static ssize_t
_getrandom_best_effort(void *buf, size_t buflen)
{
    ssize_t l;

    /* To get best-effort bytes, we would use GRND_INSECURE (and we try that
     * first). However, not all kernel versions support that, so we fallback
     * to GRND_NONBLOCK.
     *
     * Granted, this is called from a fallback path where we have no entropy
     * already, it's unlikely that GRND_NONBLOCK would succeed. Still... */
    l = _getrandom_insecure(buf, buflen);
    if (l != -EINVAL)
        return l;

    return _getrandom(buf, buflen, GRND_NONBLOCK);
}

static inline gint64
_nm_utils_timespec_to_xsec(const struct timespec *ts, gint64 xsec_per_sec)
{
    nm_assert(ts);

    if (ts->tv_sec < 0 || ts->tv_nsec < 0)
        return G_MAXINT64;

    if (ts->tv_sec > ((guint64) G_MAXINT64) || ts->tv_nsec > ((guint64) G_MAXINT64)
        || ts->tv_sec >= (G_MAXINT64 - ((gint64) ts->tv_nsec)) / xsec_per_sec)
        return G_MAXINT64;

    return (((gint64) ts->tv_sec) * xsec_per_sec)
           + (((gint64) ts->tv_nsec) / (NM_UTILS_NSEC_PER_SEC / xsec_per_sec));
}

static inline gint64
nm_utils_timespec_to_nsec(const struct timespec *ts)
{
    return _nm_utils_timespec_to_xsec(ts, NM_UTILS_NSEC_PER_SEC);
}

gint64
nm_utils_clock_gettime_nsec(clockid_t clockid)
{
    struct timespec tp;

    if (clock_gettime(clockid, &tp) != 0)
        return -NM_ERRNO_NATIVE(errno);
    return nm_utils_timespec_to_nsec(&tp);
}

pid_t
nm_utils_gettid(void)
{
    return (pid_t) syscall(SYS_gettid);
}

static void
_bad_random_init_seed(BadRandSeed *seed)
{
    const guint8 *p_at_random;
    int           seed_idx;
    GRand        *rand;

    /* g_rand_new() reads /dev/urandom too, but we already know that
     * /dev/urandom fails to give us good randomness (which is why
     * we hit the "bad random" code path). So this may not be as
     * good as we wish, but let's hope that it it does something smart
     * to give some extra entropy... */
    rand = g_rand_new();

    /* Get some seed material from a GRand. */
    for (seed_idx = 0; seed_idx < (int) G_N_ELEMENTS(seed->grand); seed_idx++)
        seed->grand[seed_idx] = g_rand_int(rand);

    /* Add an address from the heap and stack, maybe ASLR helps a bit? */
    seed->heap_ptr  = (uintptr_t) ((gpointer) rand);
    seed->stack_ptr = (uintptr_t) ((gpointer) &rand);

    g_rand_free(rand);

    /* Add the per-process, random number. */
    p_at_random = ((gpointer) getauxval(AT_RANDOM));
    if (p_at_random) {
        G_STATIC_ASSERT(sizeof(seed->auxval) == 16);
        memcpy(&seed->auxval, p_at_random, 16);
    }

    _getrandom_best_effort(seed->getrandom_buf, sizeof(seed->getrandom_buf));

    seed->now_bootime = nm_utils_clock_gettime_nsec(CLOCK_BOOTTIME);
    seed->now_real    = g_get_real_time();
    seed->pid         = getpid();
    seed->ppid        = getppid();
    seed->tid         = nm_utils_gettid();
}

static void
_bad_random_bytes(guint8 *buf, gsize n)
{
    nm_auto_free_checksum GChecksum *sum = g_checksum_new(G_CHECKSUM_SHA256);

    nm_assert(n > 0);

    /* We are in the fallback code path, where getrandom() (and /dev/urandom) failed
     * to give us good randomness. Try our best.
     *
     * Our ability to get entropy for the CPRNG is very limited and thus the overall
     * result will be bad randomness.
     *
     * Once we have some seed material, we combine GRand (which is not a cryptographically
     * secure PRNG) with some iterative sha256 hashing. It would be nice if we had
     * easy access to chacha20, but it's probably more cumbersome to fork those
     * implementations than hack a bad CPRNG by using sha256 hashing. After all, this
     * is fallback code to get *some* bad randomness. And with the inability to get a good
     * seed, any CPRNG can only give us bad randomness. */

    {
        static BadRandState gl_state;
        static GRand       *gl_rand;
        static GMutex       gl_mutex;
        NM_G_MUTEX_LOCKED(&gl_mutex);

        if (G_UNLIKELY(!gl_rand)) {
            union {
                BadRandSeed d_seed;
                guint32     d_u32[(sizeof(BadRandSeed) + 3) / 4];
            } data = {
                    .d_u32 = {0},
            };

            _bad_random_init_seed(&data.d_seed);

            gl_rand = g_rand_new_with_seed_array(data.d_u32, G_N_ELEMENTS(data.d_u32));

            g_checksum_update(sum, (const guchar *) &data, sizeof(data));
            nm_utils_checksum_get_digest(sum, gl_state.sha_digest.full);
        }

        _getrandom_best_effort(gl_state.rand_vals_getrandom, sizeof(gl_state.rand_vals_getrandom));

        gl_state.rand_vals_timestamp = nm_utils_clock_gettime_nsec(CLOCK_BOOTTIME);

        while (TRUE) {
            int i;

            gl_state.counter++;
            for (i = 0; i < G_N_ELEMENTS(gl_state.rand_vals.u32); i++)
                gl_state.rand_vals.u32[i] = g_rand_int(gl_rand);
            g_checksum_reset(sum);
            g_checksum_update(sum, (const guchar *) &gl_state, sizeof(gl_state));
            nm_utils_checksum_get_digest(sum, gl_state.sha_digest.full);

            /* gl_state.sha_digest.full and gl_state.rand_vals contain now our
             * bad random values, but they are also the state for the next iteration.
             * We must not directly expose that state to the caller, so XOR the values.
             *
             * That means, per iteration we can generate 16 bytes of bad randomness. That
             * is suitable to initialize a random UUID. */
            for (i = 0; i < (int) (NM_UTILS_CHECKSUM_LENGTH_SHA256 / 2); i++) {
                nm_assert(n > 0);
                buf[0] = gl_state.sha_digest.half_1[i] ^ gl_state.sha_digest.half_2[i]
                         ^ gl_state.rand_vals.u8[i];
                buf++;
                n--;
                if (n == 0)
                    return;
            }
        }
    }
}



/*****************************************************************************/

/**
 * nm_random_get_bytes_full:
 * @p: the buffer to fill
 * @n: the number of bytes to write to @p.
 * @out_high_quality: (out) (optional): whether the returned
 *   random bytes are of high quality.
 *
 * - will never block
 * - will always produce some numbers, but they may not
 *   be of high quality.
 * - Whether they are of high quality, you can know via @out_high_quality.
 * - will always try hard to produce high quality numbers, and on success
 *   they are as good as nm_random_get_crypto_bytes().
 */
void
nm_random_get_bytes_full(void *p, size_t n, gboolean *out_high_quality)
{
    int      fd;
    int      r;
    gboolean has_high_quality;
    ssize_t  l;

    if (n == 0) {
        NM_SET_OUT(out_high_quality, TRUE);
        return;
    }

    g_return_if_fail(p);

    again_getrandom:
    l = _getrandom(p, n, GRND_NONBLOCK);
    if (l > 0) {
        if ((size_t) l == n) {
            NM_SET_OUT(out_high_quality, TRUE);
            return;
        }
        p = ((uint8_t *) p) + l;
        n -= l;
        goto again_getrandom;
    }

    /* getrandom() failed. Fallback to read /dev/urandom. */

    if (l == -ENOSYS) {
        /* no support for getrandom(). */
        if (out_high_quality) {
            /* The caller wants to know whether we have high quality. Poll /dev/random
             * to find out. */
            has_high_quality = (_random_check_entropy(FALSE) > 0);
        } else {
            /* The value doesn't matter in this case. It will be unused. */
            has_high_quality = FALSE;
        }
    } else {
        /* Any other failure of getrandom() means we don't have high quality. */
        has_high_quality = FALSE;
        if (l == -EAGAIN) {
            /* getrandom(GRND_NONBLOCK) failed because lack of entropy. Retry with GRND_INSECURE. */
            for (;;) {
                l = _getrandom_insecure(p, n);
                if (l > 0) {
                    if ((size_t) l == n) {
                        NM_SET_OUT(out_high_quality, FALSE);
                        return;
                    }
                    p = ((uint8_t *) p) + l;
                    n -= l;
                    continue;
                }
                /* Any error. Fallback to /dev/urandom. */
                break;
            }
        }
    }

    again_open:
    fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC | O_NOCTTY);
    if (fd < 0) {
        if (errno == EINTR)
            goto again_open;
    } else {
        r = nm_utils_fd_read_loop_exact(fd, p, n, TRUE);
        nm_close(fd);
        if (r >= 0) {
            NM_SET_OUT(out_high_quality, has_high_quality);
            return;
        }
    }

    /* we failed to fill the bytes reading from /dev/urandom.
     * Fill the bits using our fallback approach (which obviously
     * cannot give high quality random).
     */
    _bad_random_bytes(p, n);
    NM_SET_OUT(out_high_quality, FALSE);
}

/**
 * nm_random_get_crypto_bytes:
 * @p: the buffer to fill
 * @n: the number of bytes to fill
 *
 * - can fail (in which case a negative number is returned
 *   and the output buffer is undefined).
 * - will block trying to get high quality random numbers.
 */
int
nm_random_get_crypto_bytes(void *p, size_t n)
{
    nm_auto_close int fd = -1;
    ssize_t           l;
    int               r;

    if (n == 0)
        return 0;

    nm_assert(p);

    again_getrandom:
    l = _getrandom(p, n, 0);
    if (l > 0) {
        if ((size_t) l == n)
            return 0;
        p = (uint8_t *) p + l;
        n -= l;
        goto again_getrandom;
    }

    if (l != -ENOSYS) {
        /* We got a failure, but getrandom seems to be working in principle. We
         * won't get good numbers. Fail. */
        return l;
    }

    /* getrandom() failed with ENOSYS. Fallback to reading /dev/urandom. */

    r = _random_check_entropy(TRUE);
    if (r < 0)
        return r;
    if (r == 0)
        return -EIO;

    fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC | O_NOCTTY);
    if (fd < 0)
        return -errno;

    return nm_utils_fd_read_loop_exact(fd, p, n, FALSE);
}

static inline void
nm_random_get_bytes(void *p, size_t n)
{
    nm_random_get_bytes_full(p, n, NULL);
}

guint64
nm_random_u64_range_full(guint64 begin, guint64 end, gboolean crypto_bytes)
{
    gboolean bad_crypto_bytes = FALSE;
    guint64  remainder;
    guint64  maxvalue;
    guint64  x;
    guint64  m;

    /* Returns a random #guint64 equally distributed in the range [@begin..@end-1].
     *
     * The function always set errno. It either sets it to zero or to EAGAIN
     * (if crypto_bytes were requested but not obtained). In any case, the function
     * will always return a random number in the requested range (worst case, it's
     * not crypto_bytes despite being requested). Check errno if you care. */

    if (begin >= end) {
        /* systemd's random_u64_range(0) is an alias for nm_random_u64().
         * Not for us. It's a caller error to request an element from an empty range. */
        return begin;
    }

    m = end - begin;

    if (m == 1) {
        x = 0;
        goto out;
    }

    remainder = G_MAXUINT64 % m;
    maxvalue  = G_MAXUINT64 - remainder;

    do
        if (crypto_bytes) {
            if (nm_random_get_crypto_bytes(&x, sizeof(x)) < 0) {
                /* Cannot get good crypto numbers. We will try our best, but fail
                 * and set errno below. */
                crypto_bytes     = FALSE;
                bad_crypto_bytes = TRUE;
                continue;
            }
        } else
            nm_random_get_bytes(&x, sizeof(x));
    while (x >= maxvalue);

    out:
    errno = bad_crypto_bytes ? EAGAIN : 0;
    return begin + (x % m);
}
