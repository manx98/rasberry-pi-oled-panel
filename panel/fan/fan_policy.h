#pragma once

// GPIO12 fan uses simple digital on/off control, not hardware PWM.
// Two thresholds and hysteresis protect the fan from rapid cycling.
namespace fan {

enum class Mode : int { Auto = 0, AlwaysOn = 1, Off = 2 };

struct Config {
    Mode mode = Mode::Auto;
    int onTemp = 55;       // Celsius, fan starts at or above this value
    int offTemp = 45;      // Celsius, fan stops at or below this value
    bool activeHigh = true; // GPIO12 high turns transistor/driver on
};

inline bool valid(const Config &c) {
    return (c.mode == Mode::Auto || c.mode == Mode::AlwaysOn || c.mode == Mode::Off)
           && c.onTemp >= 35 && c.onTemp <= 85
           && c.offTemp >= 30 && c.offTemp <= 80
           && c.onTemp - c.offTemp >= 2;
}

inline bool outputFor(const Config &c, bool wasOn, bool temperatureValid, double tempC) {
    if (c.mode == Mode::AlwaysOn) return true;
    if (c.mode == Mode::Off) return false;
    // Fail-safe: if CPU temperature becomes unavailable, turn fan on.
    if (!temperatureValid) return true;
    if (tempC >= c.onTemp) return true;
    if (tempC <= c.offTemp) return false;
    return wasOn;
}

inline bool gpioHighFor(bool fanOn, bool activeHigh) {
    return activeHigh ? fanOn : !fanOn;
}

} // namespace fan
