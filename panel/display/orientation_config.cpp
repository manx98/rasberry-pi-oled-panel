#include "panel/display/orientation_config.h"
#include "panel/display/orientation_policy.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace display {
namespace {
constexpr const char *kDir = "/var/lib/raspberry-pi-oled-panel";
constexpr const char *kFile = "/var/lib/raspberry-pi-oled-panel/display.conf";
}

OrientationConfig &OrientationConfig::instance() {
    static OrientationConfig config;
    return config;
}

void OrientationConfig::load() {
    std::ifstream file(kFile);
    if (!file) return;
    std::string line;
    if (!std::getline(file, line)) return;
    try {
        std::size_t consumed = 0;
        const int value = std::stoi(line, &consumed);
        if (consumed == line.size() && validRotation(value)) {
            degrees_.store(value, std::memory_order_relaxed);
        } else {
            std::fprintf(stderr, "Invalid OLED orientation config, using 0 degrees.\n");
        }
    } catch (...) {
        std::fprintf(stderr, "Invalid OLED orientation config, using 0 degrees.\n");
    }
}

bool OrientationConfig::setDegrees(int degrees) {
    if (!validRotation(degrees)) return false;
    if (degrees == degrees_.load(std::memory_order_relaxed)) return true;
    if (::mkdir(kDir, 0755) != 0 && errno != EEXIST) {
        std::perror("mkdir display config");
        return false;
    }
    const std::string tmp = std::string(kFile) + ".tmp." + std::to_string(getpid());
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd < 0) { std::perror("open display config"); return false; }
    const std::string contents = std::to_string(degrees) + "\n";
    std::size_t written = 0;
    while (written < contents.size()) {
        const auto count = ::write(fd, contents.data() + written, contents.size() - written);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        written += static_cast<std::size_t>(count);
    }
    bool ok = written == contents.size();
    if (ok) ok = ::fsync(fd) == 0;
    if (::close(fd) != 0) ok = false;
    if (ok) ok = ::rename(tmp.c_str(), kFile) == 0;
    if (!ok) {
        std::perror("save display orientation");
        ::unlink(tmp.c_str());
        return false;
    }
    degrees_.store(degrees, std::memory_order_relaxed);
    return true;
}
} // namespace display
