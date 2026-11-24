#include "astra/ui/list_viewport.h"
#include <cassert>
#include <cmath>
#include <vector>

static bool near(float a, float b) { return std::fabs(a - b) < 0.001f; }

int main() {
    constexpr float screen = 64;
    // Layout with headings: saved title at 0, saved profiles at 14 and 28,
    // nearby title at 42, then enough nearby APs to scroll far down.
    const std::vector<float> rows{14, 28, 56, 70, 84, 98, 112, 126, 140};
    auto viewportTop = 0.0f;
    auto scrollTo = [&](std::size_t selected) {
        const auto top = rows[selected];
        const auto heading = selected == 0 ? 0.0f : (selected == 2 ? 42.0f : -1.0f);
        viewportTop = astra::listViewportTop(viewportTop, top, top + 13, screen, heading);
        return viewportTop;
    };
    // Initially both the saved header and first profile are visible.
    assert(near(scrollTo(0), 0));
    assert(near(scrollTo(1), 0));
    // Scrolling down moves the viewport into nearby networks.
    for (std::size_t i = 2; i < rows.size(); ++i) scrollTo(i);
    assert(viewportTop > 64);
    // Going back across the nearby separator must restore the saved section.
    for (std::size_t i = rows.size() - 1; i-- > 0;) scrollTo(i);
    assert(near(viewportTop, 0)); // '已保存' title at y=0 visible again.
    assert(rows[0] + 13 <= screen);

    // If the first saved profile fits but the header does not, scroll up
    // anyway: the old camera stopped at y=14 and hid the header forever.
    assert(near(astra::listViewportTop(14, 14, 27, screen, 0), 0));
    // The same rule holds for the first nearby SSID and its separator.
    assert(near(astra::listViewportTop(60, 56, 69, screen, 42), 42));

    // Other menus, which have no section title, retain ordinary behavior.
    assert(near(astra::listViewportTop(30, 14, 27, screen), 14));
    assert(near(astra::listViewportTop(0, 100, 113, screen), 49));
    assert(near(astra::listViewportTop(49, 75, 88, screen), 49));
    // Bounds and reset behavior for refreshed lists.
    assert(near(astra::listViewportTop(110, 14, 27, screen, 0), 0));
    assert(near(astra::listViewportTop(0, 14, 27, screen, 0), 0));
    assert(near(astra::listViewportTop(100, 10, 20, 0, 0), 0));
}
