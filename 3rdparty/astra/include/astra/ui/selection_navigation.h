#pragma once

#include <cstddef>
#include <vector>

namespace wifi_list_navigation {

// A section heading is visible, but must never receive button focus.
// Reuse the same logic in the Astra selector and in host-side tests.
template <typename Item>
std::size_t firstSelectable(const std::vector<Item *> &items, std::size_t preferred = 0) {
    if (items.empty()) return 0;
    for (std::size_t i = 0; i < items.size(); ++i) {
        const auto index = (preferred % items.size() + i) % items.size();
        if (items[index] && items[index]->selectable) return index;
    }
    return 0;
}

template <typename Item>
std::size_t nextSelectable(const std::vector<Item *> &items, std::size_t current,
                           int direction) {
    if (items.empty()) return 0;
    const auto count = items.size();
    const auto origin = current % count;
    for (std::size_t i = 1; i <= count; ++i) {
        const auto index = direction < 0
            ? (origin + count - i % count) % count
            : (origin + i) % count;
        if (items[index] && items[index]->selectable) return index;
    }
    return origin;
}

} // namespace wifi_list_navigation
