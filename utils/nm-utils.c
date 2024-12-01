//
// Created by wenyiyu on 2024/10/18.
//
#include "nm-utils.h"
#include <fcntl.h>

struct IsoLangToEncodings
{
    const char *lang;
    const char *const *encodings;
};

#define LANG_ENCODINGS(l, ...) { .lang = l, .encodings = (const char *[]) { __VA_ARGS__, NULL }}

/* 5-letter language codes */
static const struct IsoLangToEncodings isoLangEntries5[] =
        {
                /* Simplified Chinese */
                LANG_ENCODINGS ("zh_cn",   "euc-cn", "gb2312", "gb18030"),         /* PRC */
                LANG_ENCODINGS ("zh_sg",   "euc-cn", "gb2312", "gb18030"),         /* Singapore */

                /* Traditional Chinese */
                LANG_ENCODINGS ("zh_tw",   "big5", "euc-tw"),                      /* Taiwan */
                LANG_ENCODINGS ("zh_hk",   "big5", "euc-tw", "big5-hkcs"),         /* Hong Kong */
                LANG_ENCODINGS ("zh_mo",   "big5", "euc-tw"),                      /* Macau */

                LANG_ENCODINGS (NULL, NULL)
        };

/* 2-letter language codes; we don't care about the other 3 in this table */
static const struct IsoLangToEncodings isoLangEntries2[] =
        {
                /* Japanese */
                LANG_ENCODINGS ("ja",      "euc-jp", "shift_jis", "iso-2022-jp"),

                /* Korean */
                LANG_ENCODINGS ("ko",      "euc-kr", "iso-2022-kr", "johab"),

                /* Thai */
                LANG_ENCODINGS ("th",      "iso-8859-11", "windows-874"),

                /* Central European */
                LANG_ENCODINGS ("hu",      "iso-8859-2", "windows-1250"),          /* Hungarian */
                LANG_ENCODINGS ("cs",      "iso-8859-2", "windows-1250"),          /* Czech */
                LANG_ENCODINGS ("hr",      "iso-8859-2", "windows-1250"),          /* Croatian */
                LANG_ENCODINGS ("pl",      "iso-8859-2", "windows-1250"),          /* Polish */
                LANG_ENCODINGS ("ro",      "iso-8859-2", "windows-1250"),          /* Romanian */
                LANG_ENCODINGS ("sk",      "iso-8859-2", "windows-1250"),          /* Slovakian */
                LANG_ENCODINGS ("sl",      "iso-8859-2", "windows-1250"),          /* Slovenian */
                LANG_ENCODINGS ("sh",      "iso-8859-2", "windows-1250"),          /* Serbo-Croatian */

                /* Cyrillic */
                LANG_ENCODINGS ("ru",      "koi8-r", "windows-1251","iso-8859-5"), /* Russian */
                LANG_ENCODINGS ("be",      "koi8-r", "windows-1251","iso-8859-5"), /* Belorussian */
                LANG_ENCODINGS ("bg",      "windows-1251","koi8-r", "iso-8859-5"), /* Bulgarian */
                LANG_ENCODINGS ("mk",      "koi8-r", "windows-1251", "iso-8859-5"),/* Macedonian */
                LANG_ENCODINGS ("sr",      "koi8-r", "windows-1251", "iso-8859-5"),/* Serbian */
                LANG_ENCODINGS ("uk",      "koi8-u", "koi8-r", "windows-1251"),    /* Ukranian */

                /* Arabic */
                LANG_ENCODINGS ("ar",      "iso-8859-6","windows-1256"),

                /* Baltic */
                LANG_ENCODINGS ("et",      "iso-8859-4", "windows-1257"),          /* Estonian */
                LANG_ENCODINGS ("lt",      "iso-8859-4", "windows-1257"),          /* Lithuanian */
                LANG_ENCODINGS ("lv",      "iso-8859-4", "windows-1257"),          /* Latvian */

                /* Greek */
                LANG_ENCODINGS ("el",      "iso-8859-7","windows-1253"),

                /* Hebrew */
                LANG_ENCODINGS ("he",      "iso-8859-8", "windows-1255"),
                LANG_ENCODINGS ("iw",      "iso-8859-8", "windows-1255"),

                /* Turkish */
                LANG_ENCODINGS ("tr",      "iso-8859-9", "windows-1254"),

                /* Table end */
                LANG_ENCODINGS (NULL, NULL)
        };
static GHashTable * langToEncodings5 = NULL;
static GHashTable * langToEncodings2 = NULL;

static void
init_lang_to_encodings_hash (void)
{
    struct IsoLangToEncodings *enc;

    if (G_UNLIKELY (langToEncodings5 == NULL)) {
        /* Five-letter codes */
        enc = (struct IsoLangToEncodings *) &isoLangEntries5[0];
        langToEncodings5 = g_hash_table_new (g_str_hash, g_str_equal);
        while (enc->lang) {
            g_hash_table_insert (langToEncodings5, (gpointer) enc->lang,
                                 (gpointer) enc->encodings);
            enc++;
        }
    }

    if (G_UNLIKELY (langToEncodings2 == NULL)) {
        /* Two-letter codes */
        enc = (struct IsoLangToEncodings *) &isoLangEntries2[0];
        langToEncodings2 = g_hash_table_new (g_str_hash, g_str_equal);
        while (enc->lang) {
            g_hash_table_insert (langToEncodings2, (gpointer) enc->lang,
                                 (gpointer) enc->encodings);
            enc++;
        }
    }
}

static gboolean
get_encodings_for_lang (const char *lang, const char *const **encodings)
{
    gs_free char *tmp_lang = NULL;

    g_return_val_if_fail (lang, FALSE);
    g_return_val_if_fail (encodings, FALSE);

    init_lang_to_encodings_hash ();

    if ((*encodings = g_hash_table_lookup (langToEncodings5, lang)))
        return TRUE;

    /* Truncate tmp_lang to length of 2 */
    if (strlen (lang) > 2) {
        tmp_lang = g_strdup (lang);
        tmp_lang[2] = '\0';
        if ((*encodings = g_hash_table_lookup (langToEncodings2, tmp_lang)))
            return TRUE;
    }

    return FALSE;
}

static const char *const *
get_system_encodings (void)
{
    static const char *const *cached_encodings;
    static char *default_encodings[4];
    const char *const *encodings = NULL;
    char *lang;

    if (cached_encodings)
        return cached_encodings;

    /* Use environment variables as encoding hint */
    lang = getenv ("LC_ALL");
    if (!lang)
        lang = getenv ("LC_CTYPE");
    if (!lang)
        lang = getenv ("LANG");
    if (lang) {
        char *dot;

        lang = g_ascii_strdown (lang, -1);
        if ((dot = strchr (lang, '.')))
            *dot = '\0';

        get_encodings_for_lang (lang, &encodings);
        g_free (lang);
    }
    if (!encodings) {
        g_get_charset ((const char **) &default_encodings[0]);
        default_encodings[1] = "iso-8859-1";
        default_encodings[2] = "windows-1251";
        default_encodings[3] = NULL;
        encodings = (const char *const *) default_encodings;
    }

    cached_encodings = encodings;
    return cached_encodings;
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
nm_utils_ssid_to_utf8 (const guint8 *ssid, gsize len)
{
    const char *const *encodings;
    const char *const *e;
    char *converted = NULL;

    g_return_val_if_fail (ssid != NULL, NULL);

    if (g_utf8_validate ((const char *) ssid, len, NULL))
        return g_strndup ((const char *) ssid, len);

    encodings = get_system_encodings ();

    for (e = encodings; *e; e++) {
        converted = g_convert ((const char *) ssid, len, "UTF-8", *e, NULL, NULL, NULL);
        if (converted)
            break;
    }

    if (!converted) {
        converted = g_convert_with_fallback ((const char *) ssid, len,
                                             "UTF-8", encodings[0], "?", NULL, NULL, NULL);
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

        converted = g_strndup ((const char *) ssid, len);
        g_strcanon (converted, valid_chars, '?');
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

const char * nm_active_connection_state_reason_to_string(NMActiveConnectionStateReason value){
    switch (value) {
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_UNKNOWN, N_("Unknown reason"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_NONE,
                             N_("The connection was disconnected"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_USER_DISCONNECTED,
                             N_("Disconnected by user"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_DEVICE_DISCONNECTED,
                             N_("The base network connection was interrupted"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_SERVICE_STOPPED,
                             N_("The VPN service stopped unexpectedly"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_IP_CONFIG_INVALID,
                             N_("The VPN service returned invalid configuration"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_CONNECT_TIMEOUT,
                             N_("The connection attempt timed out"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_SERVICE_START_TIMEOUT,
                             N_("The VPN service did not start in time"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_SERVICE_START_FAILED,
                             N_("The VPN service failed to start"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_NO_SECRETS, N_("No valid secrets"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_LOGIN_FAILED, N_("Invalid secrets"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_CONNECTION_REMOVED,
                             N_("The connection was removed"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_DEPENDENCY_FAILED,
                             N_("Master connection failed"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_DEVICE_REALIZE_FAILED,
                             N_("Could not create a software link"))
        NM_UTILS_LOOKUP_ITEM(NM_ACTIVE_CONNECTION_STATE_REASON_DEVICE_REMOVED,
                             N_("The device disappeared"))
    }
    /* TRANSLATORS: Unknown reason for a connection state change (NMActiveConnectionStateReason) */
    NM_UTILS_LOOKUP_DEFAULT(N_("Unknown"));
}

NMActiveConnectionState
nmc_activation_get_effective_state(NMActiveConnection *active,
                                   NMDevice           *device,
                                   const char        **reason)
{
    NMActiveConnectionState       ac_state;
    NMActiveConnectionStateReason ac_reason;
    NMDeviceState                 dev_state  = NM_DEVICE_STATE_UNKNOWN;
    NMDeviceStateReason           dev_reason = NM_DEVICE_STATE_REASON_UNKNOWN;

    g_return_val_if_fail(active, NM_ACTIVE_CONNECTION_STATE_UNKNOWN);
    g_return_val_if_fail(reason, NM_ACTIVE_CONNECTION_STATE_UNKNOWN);

    *reason   = NULL;
    ac_reason = nm_active_connection_get_state_reason(active);

    if (device) {
        dev_state  = nm_device_get_state(device);
        dev_reason = nm_device_get_state_reason(device);
    }

    ac_state = nm_active_connection_get_state(active);
    switch (ac_state) {
        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
            if (!device || ac_reason != NM_ACTIVE_CONNECTION_STATE_REASON_DEVICE_DISCONNECTED
                || nm_device_get_active_connection(device) != active) {
                /* (1)
                 * - we have no device,
                 * - or, @ac_reason is specific
                 * - or, @device no longer references the current @active
                 * >> we complete with @ac_reason. */
                *reason = nm_active_connection_state_reason_to_string(ac_reason);
            } else if (dev_state <= NM_DEVICE_STATE_DISCONNECTED
                       || dev_state >= NM_DEVICE_STATE_FAILED) {
                /* (2)
                 * - not (1)
                 * - and, the device is no longer in an activated state,
                 * >> we complete with @dev_reason. */
                *reason = nmc_device_reason_to_string(dev_reason);
            } else {
                /* (3)
                 * we wait for the device go disconnect. We will get a better
                 * failure reason from the device (2). */
                return NM_ACTIVE_CONNECTION_STATE_UNKNOWN;
            }
            break;
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
            /* activating controller connection does not automatically activate any ports, so their
             * active connection state will not progress beyond ACTIVATING state.
             * Monitor the device instead. */
            if (device
                && (NM_IS_DEVICE_BOND(device) || NM_IS_DEVICE_TEAM(device)
                    || NM_IS_DEVICE_BRIDGE(device))
                && dev_state >= NM_DEVICE_STATE_IP_CONFIG && dev_state <= NM_DEVICE_STATE_ACTIVATED) {
                *reason = "controller waiting for ports";
                return NM_ACTIVE_CONNECTION_STATE_ACTIVATED;
            }
            break;
        default:
            break;
    }

    return ac_state;
}
