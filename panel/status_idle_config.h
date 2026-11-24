#pragma once

#include <atomic>
#include <string>

namespace status_idle {

// 0 disables automatic hiding; 5..300 seconds in five-second increments.
// Persisted independently from OLED rotation and fan thresholds.
inline bool validSeconds(int value) {
    return value >= 0 && value <= 300 && value % 5 == 0;
}

class Config {
public:
    explicit Config(std::string path = "/var/lib/raspberry-pi-oled-panel/status-idle.conf");
    static Config &instance();

    void load();
    int seconds() const { return seconds_.load(std::memory_order_relaxed); }
    // Save atomically before applying, leave previous setting on failure.
    bool setSeconds(int seconds);

private:
    std::string path_;
    std::atomic<int> seconds_{30};
};

} // namespace status_idle
