#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sh1106 {
// Clear all 132 columns of controller RAM, including the hidden columns,
// so the operation is independent of the current segment remap/orientation.
template <typename Write>
bool clearDisplay(Write write) {
    const std::array<uint8_t, 2> off{0x00, 0xae};
    if (!write(off.data(), off.size())) return false;
    std::array<uint8_t, 133> pixels{};
    pixels[0] = 0x40; // I2C data control byte; remaining bytes are zero.
    for (uint8_t page = 0; page < 8; ++page) {
        const std::array<uint8_t, 4> address{
            0x00, static_cast<uint8_t>(0xb0 | page), 0x00, 0x10};
        if (!write(address.data(), address.size()) ||
            !write(pixels.data(), pixels.size())) return false;
    }
    return true;
}
} // namespace sh1106
