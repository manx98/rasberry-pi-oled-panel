#include "sh1106_hal/clear_display.h"
#include <cassert>
#include <vector>

int main() {
    std::vector<std::vector<uint8_t>> writes;
    assert(sh1106::clearDisplay([&](const uint8_t *data, std::size_t size) {
        writes.emplace_back(data, data + size);
        return true;
    }));
    assert(writes.size() == 17);
    assert((writes[0] == std::vector<uint8_t>{0x00, 0xae}));
    for (int page = 0; page < 8; ++page) {
        assert((writes[1 + page * 2] == std::vector<uint8_t>{
            0x00, static_cast<uint8_t>(0xb0 | page), 0x00, 0x10}));
        const auto &data = writes[2 + page * 2];
        assert(data.size() == 133 && data[0] == 0x40);
        for (std::size_t i = 1; i < data.size(); ++i) assert(data[i] == 0);
    }
    for (int fail = 1; fail <= 17; ++fail) {
        int calls = 0;
        assert(!sh1106::clearDisplay([&](const uint8_t *, std::size_t) {
            return ++calls != fail;
        }));
        assert(calls == fail);
    }
}
