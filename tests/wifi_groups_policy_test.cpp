#include "panel/wifi_groups_policy.h"
#include <cassert>
#include <list>
#include <string>

struct Saved { std::string ssid, bssid, uuid; };
struct Nearby { std::string SSID, BSS; };
int main() {
    const std::list<Saved> saved{
        {"Office", "", "uuid-1"},
        {"Lab", "aa:BB:cc:DD:ee:ff", "uuid-2"},
        {"Lab", "11:22:33:44:55:66", "uuid-3"},
        {"", "", "uuid-hidden"}
    };
    assert(wifi_groups::isSavedNetwork(saved, Nearby{"Office", "00:01:02:03:04:05"}));
    assert(wifi_groups::isSavedNetwork(saved, Nearby{"Lab", "AA:bb:CC:dd:EE:FF"}));
    assert(wifi_groups::isSavedNetwork(saved, Nearby{"Lab", "11:22:33:44:55:66"}));
    assert(!wifi_groups::isSavedNetwork(saved, Nearby{"Lab", "00:00:00:00:00:00"}));
    assert(!wifi_groups::isSavedNetwork(saved, Nearby{"Guest", "aa:BB:cc:DD:ee:ff"}));
    assert(!wifi_groups::isSavedNetwork(saved, Nearby{"", ""}));
    assert(!wifi_groups::isSavedNetwork(std::list<Saved>{}, Nearby{"Office", ""}));
    // Distinct UUIDs must be preserved, even when the SSID is shared.
    assert(saved.size() == 4);
}
