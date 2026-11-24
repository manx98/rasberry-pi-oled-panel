#include <iostream>
#include <string>

// Replaces /usr/bin/nmcli only in the transport unit test. No real network
// commands are ever executed by this test.
int main(int argc, char **argv) {
    bool asked = false;
    bool ifname = false;
    bool containsSSID = false;
    bool isSaved = false;
    bool isForget = false;
    bool hasUUID = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--ask") asked = true;
        if (arg == "TestSSID") containsSSID = true;
        if (arg == "wlan-test") ifname = true;
        if (arg == "up" && i >= 2 && std::string(argv[i - 1]) == "connection") isSaved = true;
        if (arg == "delete" && i >= 2 && std::string(argv[i - 1]) == "connection") isForget = true;
        if (arg == "uuid" && i + 1 < argc && std::string(argv[i + 1]) == "11111111-2222-3333-4444-555555555555")
            hasUUID = true;
        if (arg.find("abcD123!@") != std::string::npos) return 5; // secret never in argv
    }
    if (isForget) return hasUUID ? 0 : 9;
    if (isSaved) return hasUUID && ifname ? 0 : 10;
    if (!containsSSID || !ifname) return 6;
    if (asked) {
        std::cout << "Password (802-11-wireless-security.psk): " << std::flush;
        std::string secret;
        if (!std::getline(std::cin, secret)) return 7;
        return secret == "abcD123!@" ? 0 : 8;
    }
    return 0;
}
