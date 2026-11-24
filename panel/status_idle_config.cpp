#include "panel/status_idle_config.h"

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace status_idle {

Config::Config(std::string path) : path_(std::move(path)) {}

Config &Config::instance() {
    static Config config;
    return config;
}

void Config::load() {
    std::ifstream in(path_);
    if (!in) return; // No config file means the original 30-second default.
    std::string line;
    if (!std::getline(in, line)) return;
    try {
        std::size_t consumed = 0;
        const int value = std::stoi(line, &consumed);
        if (consumed == line.size() && validSeconds(value)) {
            seconds_.store(value, std::memory_order_relaxed);
            return;
        }
    } catch (...) {}
    std::fprintf(stderr, "Invalid status idle timeout; retaining %d seconds.\n", seconds());
}

bool Config::setSeconds(int seconds) {
    if (!validSeconds(seconds)) return false;
    if (seconds_.load(std::memory_order_relaxed) == seconds) return true;

    const auto slash = path_.find_last_of('/');
    if (slash != std::string::npos && slash != 0) {
        const auto dir = path_.substr(0, slash);
        if (::mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST) {
            std::perror("create status idle config directory");
            return false;
        }
    }
    std::string tmp = path_ + ".tmp.XXXXXX";
    const int fd = ::mkstemp(tmp.data());
    if (fd < 0) {
        std::perror("open temporary status idle config");
        return false;
    }
    ::fchmod(fd, 0644);
    const std::string contents = std::to_string(seconds) + "\n";
    std::size_t written = 0;
    while (written < contents.size()) {
        const auto count = ::write(fd, contents.data() + written, contents.size() - written);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        written += static_cast<std::size_t>(count);
    }
    bool okay = written == contents.size();
    if (okay) okay = ::fsync(fd) == 0;
    if (::close(fd) != 0) okay = false;
    if (okay) okay = ::rename(tmp.c_str(), path_.c_str()) == 0;
    if (!okay) {
        std::perror("persist status idle timeout");
        ::unlink(tmp.c_str());
        return false;
    }
    seconds_.store(seconds, std::memory_order_relaxed);
    return true;
}

} // namespace status_idle
