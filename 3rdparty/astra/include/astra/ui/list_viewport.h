#pragma once

#include <algorithm>

namespace astra {

// Pure viewport calculation for vertical menus. The caller supplies the
// preceding section title's top edge when the selected row is its first
// selectable child. Both rows should then reappear together on scroll-up.
inline float listViewportTop(float currentTop, float itemTop, float itemBottom,
                             float screenHeight, float sectionTop = -1.0f) {
    if (screenHeight <= 0) return 0;
    const float firstVisible = sectionTop >= 0 ? std::min(sectionTop, itemTop) : itemTop;
    if (firstVisible < currentTop) {
        return std::max(0.0f, firstVisible);
    }
    if (itemBottom > currentTop + screenHeight) {
        return std::max(0.0f, itemBottom - screenHeight);
    }
    return std::max(0.0f, currentTop);
}

} // namespace astra
