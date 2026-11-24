#pragma once

#include <string>

namespace RPI {

// Executes nmcli without invoking a shell or placing the password in argv.
// Passphrases are supplied on the child's private stdin via `--ask`.
enum class WifiCommandResult {
    Success,
    Failed,
    Timeout,
    Unavailable
};

WifiCommandResult wifiConnectNew(const std::string &iface, const std::string &ssid,
                                 const std::string &bssid, const std::string &password);
// UUID avoids accidentally activating a different profile with the same name.
WifiCommandResult wifiConnectSaved(const std::string &iface, const std::string &uuid);
WifiCommandResult wifiDisconnect(const std::string &iface);
WifiCommandResult wifiForgetSaved(const std::string &connectionUuid);

} // namespace RPI
