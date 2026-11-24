#pragma once

#include <string>

// On cancellation, leaves password empty. Never displays or logs plaintext.
bool inputWifiPassword(const std::string &ssid, bool legacyWep, std::string &password);

// Defensive confirmation for destructive Wi-Fi actions like disconnect/forget.
bool confirmWifiOperation(const std::string &ssid, const std::string &action);
