#include "panel/network_list_layout.h"

#include <cassert>
#include <map>
#include <string>

int main() {
    using network_list::compactRow;
    const auto down = compactRow("eth0", "", "");
    assert(down.name == "- eth0");
    assert(down.address.empty()); // A DOWN NIC still occupies one row.

    const auto loopback = compactRow("lo", "127.0.0.1", "::1");
    assert(loopback.name == "- lo");
    assert(loopback.address == "  127.0.0.1");

    const auto wifi = compactRow("wlan0", "192.168.1.6",
                                 "2409:8a3c:434:d590:e65f:1ff:fe50:19d7");
    assert(wifi.name == "- wlan0");
    assert(wifi.address == "  192.168.1.6");

    const auto v6only = compactRow("eth1", "", "fe80::1");
    assert(v6only.address == "  IPv6: fe80::1");

    // With the existing 9px font and compact 2px row gap, all three real
    // interfaces from the Pi fit in a 64px OLED list without clipping.
    constexpr float fontHeight = 9;
    constexpr float lineGap = 1;
    constexpr float rowGap = 2;
    const auto pitch = [&](const network_list::Row &row) {
        return fontHeight + (!row.address.empty() ? fontHeight + lineGap : 0) + rowGap;
    };
    assert(pitch(down) + pitch(loopback) + pitch(wifi) <= 64);
    // A device with a long IPv6 address still occupies at most two lines.
    assert(pitch(v6only) == fontHeight * 2 + lineGap + rowGap);
}
