#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace wifi_groups {

inline bool sameBssid(const std::string &a, const std::string &b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(),
        [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
}

template <typename Saved, typename AccessPoint>
bool matchesSavedProfile(const Saved &profile, const AccessPoint &accessPoint) {
    return !profile.ssid.empty() && profile.ssid == accessPoint.SSID
           && (profile.bssid.empty() || sameBssid(profile.bssid, accessPoint.BSS));
}

template <typename SavedCollection, typename AccessPoint>
bool isSavedNetwork(const SavedCollection &saved, const AccessPoint &accessPoint) {
    return std::any_of(saved.begin(), saved.end(), [&](const auto &profile) {
        return matchesSavedProfile(profile, accessPoint);
    });
}

} // namespace wifi_groups
