#pragma once

#include <atomic>

namespace display {
class OrientationConfig {
public:
    static OrientationConfig &instance();

    // Load saved orientation before the OLED and button subsystem starts.
    void load();
    int degrees() const { return degrees_.load(std::memory_order_relaxed); }
    // Persist before applying; on IO failure, leave prior orientation intact.
    bool setDegrees(int degrees);

private:
    OrientationConfig() = default;
    std::atomic<int> degrees_{0};
};
} // namespace display
