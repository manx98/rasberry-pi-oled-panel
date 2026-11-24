#include "panel/fan/fan_controller.h"

#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace fan {
namespace {
constexpr char kConfigDir[] = "/var/lib/raspberry-pi-oled-panel";
constexpr char kConfigFile[] = "/var/lib/raspberry-pi-oled-panel/fan.conf";
constexpr char kThermalPath[] = "/sys/class/thermal/thermal_zone0/temp";

bool readTemperature(double &temp) {
    std::ifstream input(kThermalPath);
    long millidegrees = -1;
    if (!(input >> millidegrees) || millidegrees < 0 || millidegrees > 120000) return false;
    temp = static_cast<double>(millidegrees) / 1000.0;
    return true;
}

int acquireGpio12(bool activeHigh) {
    int chip = ::open("/dev/gpiochip0", O_RDONLY | O_CLOEXEC);
    if (chip < 0) throw std::system_error(errno, std::generic_category(), "open GPIO chip");
    gpio_v2_line_request req{};
    req.offsets[0] = 12; // BCM GPIO 12, physical header pin 32
    req.num_lines = 1;
    std::strncpy(req.consumer, "oled-panel-fan", sizeof(req.consumer) - 1);
    req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    req.config.num_attrs = 1;
    req.config.attrs[0].mask = 1;
    req.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
    // Set the initial state to fan ON before the first thermal sample.
    // A disconnected temperature sensor must never silently disable cooling.
    req.config.attrs[0].attr.values = gpioHighFor(true, activeHigh) ? 1 : 0;
    int error = ioctl(chip, GPIO_V2_GET_LINE_IOCTL, &req);
    int saved = errno;
    close(chip);
    if (error < 0) throw std::system_error(saved, std::generic_category(), "claim GPIO12 fan");
    return req.fd;
}

bool writeFanLevel(int fd, bool high) {
    gpio_v2_line_values v{};
    v.mask = 1;
    v.bits = high ? 1 : 0;
    return ioctl(fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &v) == 0;
}

} // namespace

Controller &Controller::instance() {
    static Controller self;
    return self;
}

Controller::~Controller() { stop(); }

void Controller::load() {
    std::ifstream in(kConfigFile);
    if (!in) return;
    Config next = status_.config;
    std::string line;
    try {
        while (std::getline(in, line)) {
            const auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            const auto key = line.substr(0, pos);
            const auto number = std::stoi(line.substr(pos + 1));
            if (key == "mode") next.mode = static_cast<Mode>(number);
            else if (key == "on_temp") next.onTemp = number;
            else if (key == "off_temp") next.offTemp = number;
            else if (key == "active_high") {
                if (number != 0 && number != 1) return;
                next.activeHigh = number == 1;
            }
        }
        if (valid(next)) status_.config = next;
        else std::fprintf(stderr, "Invalid fan config, using defaults.\n");
    } catch (const std::exception &e) {
        std::fprintf(stderr, "Invalid fan config (%s), using defaults.\n", e.what());
    }
}

bool Controller::save(const Config &next) const {
    if (mkdir(kConfigDir, 0755) != 0 && errno != EEXIST) {
        std::perror("mkdir fan settings");
        return false;
    }
    const std::string tempFile = std::string(kConfigFile) + ".tmp." + std::to_string(getpid());
    const int fd = ::open(tempFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) { std::perror("open fan settings"); return false; }
    const std::string content = "mode=" + std::to_string(static_cast<int>(next.mode))
        + "\non_temp=" + std::to_string(next.onTemp)
        + "\noff_temp=" + std::to_string(next.offTemp)
        + "\nactive_high=" + std::to_string(next.activeHigh ? 1 : 0) + "\n";
    ssize_t written = 0;
    while (written < static_cast<ssize_t>(content.size())) {
        const ssize_t count = write(fd, content.data() + written, content.size() - written);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        written += count;
    }
    const bool good = written == static_cast<ssize_t>(content.size()) && fsync(fd) == 0;
    close(fd);
    if (!good || rename(tempFile.c_str(), kConfigFile) != 0) {
        std::perror("persist fan settings");
        unlink(tempFile.c_str());
        return false;
    }
    return true;
}

void Controller::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) return;
    load();
    if (access(kConfigFile, F_OK) != 0 && !save(status_.config)) {
        throw std::runtime_error("cannot initialize fan settings");
    }
    gpioFd_ = acquireGpio12(status_.config.activeHigh);
    status_.fanOn = true;
    status_.gpioOkay = true;
    running_ = true;
    try { thread_ = std::thread(&Controller::loop, this); }
    catch (...) {
        running_ = false;
        close(gpioFd_);
        gpioFd_ = -1;
        throw;
    }
}

void Controller::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) return;
        running_ = false;
    }
    wake_.notify_all();
    if (thread_.joinable()) thread_.join();
    std::lock_guard<std::mutex> lock(mutex_);
    if (gpioFd_ >= 0) close(gpioFd_);
    gpioFd_ = -1;
    status_.gpioOkay = false;
}

Config Controller::config() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_.config;
}

Status Controller::status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

bool Controller::update(const Config &next) {
    if (!valid(next)) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!save(next)) return false;
    status_.config = next;
    wake_.notify_all();
    return true;
}

void Controller::loop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_) {
        Config c = status_.config;
        lock.unlock();
        double temp = 0;
        const bool tempOkay = readTemperature(temp);
        lock.lock();
        // Use the latest configuration even if changed during a sensor read.
        c = status_.config;
        const bool desired = outputFor(c, status_.fanOn, tempOkay, temp);
        const bool level = gpioHighFor(desired, c.activeHigh);
        if (!writeFanLevel(gpioFd_, level)) {
            status_.gpioOkay = false;
            std::perror("set GPIO12 fan level");
        } else {
            status_.gpioOkay = true;
            status_.fanOn = desired;
        }
        status_.temperatureValid = tempOkay;
        status_.temperatureC = temp;
        wake_.wait_for(lock, std::chrono::seconds(1));
    }
}

} // namespace fan
