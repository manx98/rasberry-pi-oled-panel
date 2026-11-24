#include "astra/ui/selection_navigation.h"
#include <cassert>
#include <vector>

struct Row { bool selectable; };
int main() {
    Row savedHeader{false}, savedA{true}, savedB{true};
    Row nearbyHeader{false}, nearbyA{true}, empty{false};
    std::vector<Row *> rows{&savedHeader, &savedA, &savedB, &nearbyHeader, &nearbyA};
    using namespace wifi_list_navigation;
    assert(firstSelectable(rows) == 1);
    assert(firstSelectable(rows, 3) == 4);
    assert(nextSelectable(rows, 1, 1) == 2);
    assert(nextSelectable(rows, 2, 1) == 4); // Skip nearby separator.
    assert(nextSelectable(rows, 4, 1) == 1); // Wrap past saved separator.
    assert(nextSelectable(rows, 1, -1) == 4);
    assert(nextSelectable(rows, 4, -1) == 2);
    std::vector<Row *> onlyOne{&savedHeader, &savedA, &nearbyHeader};
    assert(firstSelectable(onlyOne) == 1);
    assert(nextSelectable(onlyOne, 1, 1) == 1);
    assert(nextSelectable(onlyOne, 1, -1) == 1);
    std::vector<Row *> noActions{&savedHeader, &empty, &nearbyHeader};
    assert(firstSelectable(noActions) == 0);
    assert(nextSelectable(noActions, 0, 1) == 0);
    assert(nextSelectable(noActions, 0, -1) == 0);
    std::vector<Row *> none;
    assert(firstSelectable(none) == 0);
    assert(nextSelectable(none, 0, 1) == 0);
}
