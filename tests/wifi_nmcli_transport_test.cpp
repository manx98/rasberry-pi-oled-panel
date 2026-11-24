#include "utils/wifi_nmcli.h"
#include <cassert>

int main() {
    using RPI::WifiCommandResult;
    // Mock simulates the actual --ask prompt; password arrives through a
    // private pseudo terminal, never in argv or the test's output.
    assert(RPI::wifiConnectNew("wlan-test", "TestSSID", "02:11:22:33:44:55", "abcD123!@")
           == WifiCommandResult::Success);
    assert(RPI::wifiConnectNew("wlan-test", "TestSSID", "", "")
           == WifiCommandResult::Success);
    assert(RPI::wifiConnectNew("", "TestSSID", "", "abcD123!@")
           == WifiCommandResult::Failed);
    // Saved profile commands use the unambiguous UUID, not a display name.
    assert(RPI::wifiConnectSaved("wlan-test", "11111111-2222-3333-4444-555555555555")
           == WifiCommandResult::Success);
    assert(RPI::wifiConnectSaved("wlan-test", "") == WifiCommandResult::Failed);
    assert(RPI::wifiForgetSaved("11111111-2222-3333-4444-555555555555")
           == WifiCommandResult::Success);
}
