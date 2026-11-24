#include "astra/ui/navigation_idle_timer.h"
#include <cassert>

int main() {
    astra::NavigationIdleTimer timer;
    int root = 0;
    int status = 0;
    int wifi = 0;
    assert(!timer.shouldHide(&root, false, false, 0));
    assert(!timer.shouldHide(&status, true, false, 1000));
    assert(!timer.shouldHide(&status, true, false, 30999));
    assert(timer.shouldHide(&status, true, false, 31000));
    assert(timer.shouldHide(&status, true, false, 45000));
    assert(!timer.shouldHide(&status, true, true, 45001));
    assert(!timer.shouldHide(&status, true, false, 75000));
    assert(timer.shouldHide(&status, true, false, 75001));
    // Leaving/reentering status resets idle countdown.
    assert(!timer.shouldHide(&wifi, false, false, 90000));
    assert(!timer.shouldHide(&status, true, false, 90001));
    assert(!timer.shouldHide(&status, true, true, 120001));
    assert(timer.shouldHide(&status, true, false, 150001));
    // Custom 5-second timeout.
    // An already idle page stays hidden when its threshold is shorter.
    assert(timer.shouldHide(&status, true, false, 150002, 5000));
    // A key restores the navigation elements and restarts the new interval.
    assert(!timer.shouldHide(&status, true, true, 150003, 5000));
    assert(!timer.shouldHide(&status, true, false, 155002, 5000));
    assert(timer.shouldHide(&status, true, false, 155003, 5000));
    // Zero disables hiding even with a very long idle duration.
    assert(!timer.shouldHide(&status, true, false, 500000, 0));
    assert(!timer.shouldHide(&status, true, false, 504999, 5000));
    assert(timer.shouldHide(&status, true, false, 505000, 5000));
    // Other pages never hide navigation widgets, regardless of inactivity.
    assert(!timer.shouldHide(&wifi, false, false, 150002));
    assert(!timer.shouldHide(&wifi, false, false, 250002));
}
