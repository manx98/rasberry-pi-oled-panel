#pragma once

#include "panel/fan/fan_policy.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace fan {

struct Status {
    Config config;
    bool fanOn = false;
    bool temperatureValid = false;
    double temperatureC = 0;
    bool gpioOkay = false;
};

class Controller {
public:
    static Controller &instance();
    void start();
    void stop();
    Config config() const;
    Status status() const;
    // Validate, durably save, and apply. Returns false if invalid or IO fails.
    bool update(const Config &next);

private:
    Controller() = default;
    ~Controller();
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;
    void loop();
    bool save(const Config &next) const;
    void load();

    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::thread thread_;
    bool running_ = false;
    int gpioFd_ = -1;
    Status status_{};
};

} // namespace fan
