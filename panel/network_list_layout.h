#pragma once

#include <string>

namespace network_list {

// A 128x64 list row must remain compact even if an interface has multiple
// IPv6 addresses. The complete address belongs in a detail view, not in the
// selection list used for enabling/disabling network interfaces.
struct Row {
    std::string name;
    std::string address;
};

inline Row compactRow(const std::string &name, const std::string &ipv4,
                      const std::string &ipv6) {
    Row row{"- " + name, ""};
    if (!ipv4.empty()) {
        row.address = "  " + ipv4;
    } else if (!ipv6.empty()) {
        row.address = "  IPv6: " + ipv6;
    }
    return row;
}

} // namespace network_list
