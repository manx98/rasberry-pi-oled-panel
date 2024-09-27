#include "utils.h"
#include <cstdio>
#include <spdlog/spdlog.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>


namespace RPI {
    static int exec_cmd(const std::string &cmd, std::string &result) {
        auto file = popen((cmd + " 2>&1").c_str(), "r");
        if (!file) {
            spdlog::error("failed to run cmd: {}", cmd);
            return -1;
        }
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file) != nullptr) {
            result.append(buffer);
        }
        return pclose(file);
    }

    WifiConnectInfo *getWifiConnectInfo(const std::string &interface_name) {
        auto info = new WifiConnectInfo;
        std::string result;
        if (exec_cmd("iw dev " + interface_name + " link", result)) {
            spdlog::error("Failed to get wifi connect info: {}", result);
            return {};
        }
        size_t end_pos;
        size_t start_pos;
        start_pos = result.find("SSID: ");
        if (start_pos == std::string::npos) {
            goto err;
        }
        end_pos = result.find('\n', start_pos);
        if (end_pos == std::string::npos) {
            goto err;
        }
        info->SSID = result.substr(start_pos + 6, end_pos - start_pos - 6);
        start_pos = result.find("signal: ");
        if (start_pos == std::string::npos) {
            goto err;
        }
        end_pos = result.find(" dBm", start_pos);
        if (end_pos == std::string::npos) {
            goto err;
        }
        info->signal = std::stof(result.substr(start_pos + 7, end_pos - start_pos - 7));
        start_pos = result.find("RX: ");
        if (start_pos == std::string::npos) {
            goto err;
        }
        end_pos = result.find(" bytes", start_pos);
        if (end_pos == std::string::npos) {
            goto err;
        }
        info->RX = std::stoll(result.substr(start_pos + 4, end_pos - start_pos - 4));
        start_pos = result.find("TX: ");
        if (start_pos == std::string::npos) {
            goto err;
        }
        end_pos = result.find(" bytes", start_pos);
        if (end_pos == std::string::npos) {
            goto err;
        }
        info->TX = std::stoll(result.substr(start_pos + 4, end_pos - start_pos - 4));
        spdlog::info("SSID: {}, signal: {}, RX: {}, TX: {}", info->SSID, info->signal, info->RX, info->TX);
        return info;
        err:
        delete info;
        return nullptr;
    }

    std::list<WifiInfo> getWifiList(const std::string &interface_name) {
        std::string result;
        if (exec_cmd("iw dev " + interface_name + " scan", result)) {
            spdlog::error("Failed to get wifi list: {}", result);
            return {};
        }
        std::list<WifiInfo> wifi_info_list;
        size_t last_pos = 0;
        while (true) {
            WifiInfo info;
            auto pos = result.find("SSID: ", last_pos);
            if (pos == std::string::npos) {
                break;
            }
            auto bss_pos = result.rfind("BSS ", pos);
            if (bss_pos == std::string::npos) {
                break;
            }
            auto bss_pos_end = result.find('(', bss_pos);
            if (bss_pos_end == std::string::npos) {
                break;
            }
            info.BSS = result.substr(bss_pos + 4, bss_pos_end - bss_pos - 4);
            pos += 6;
            last_pos = result.find('\n', pos);
            if (last_pos == std::string::npos) {
                break;
            }
            info.SSID = result.substr(pos, last_pos - pos);
            pos = result.find("signal: ", last_pos);
            if (pos == std::string::npos) {
                break;
            }
            pos += 8;
            last_pos = result.find(" dBm", pos);
            if (last_pos == std::string::npos) {
                break;
            }
            info.signal = std::stof(result.substr(pos, last_pos - pos));
            spdlog::info("BSS: {}, SSID: {}, signal: {}", info.BSS, info.SSID, info.signal);
            wifi_info_list.emplace_back(info);
        }
        return wifi_info_list;
    }

    std::list<std::string> getWifiDevices() {
        std::string result;
        if (exec_cmd("iw dev", result)) {
            spdlog::error("Failed to get wifi devices: {}", result);
            return {};
        }
        std::list<std::string> interface_list;
        size_t last_pos = 0;
        while (true) {
            auto pos = result.find("Interface", last_pos);
            if (pos == std::string::npos) {
                break;
            }
            pos += 10;
            last_pos = result.find('\n', pos);
            if (last_pos == std::string::npos) {
                break;
            }
            auto name = result.substr(pos, last_pos - pos);
            spdlog::info("Interface: {}", name);
            interface_list.emplace_back(name);
        }
        return interface_list;
    }

    bool setIpLinkUp(const char *interface_name, bool up) {
        struct ifreq ifr{};
        int sockfd;

        // 打开一个套接字
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            spdlog::error("Failed to open socket");
            return false;
        }

        // 设置网卡名称
        strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);

        // 获取网卡当前状态
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
            spdlog::error("Failed to get interface flags");
            close(sockfd);
            return false;
        }

        // 检查网卡是否已经启用
        if (ifr.ifr_flags & IFF_UP) {
            if (!up) {
                // 禁用网卡
                ifr.ifr_flags &= ~IFF_UP;
                if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
                    spdlog::error("Failed to disable interface");
                    close(sockfd);
                    return false;
                }
            } else {
                // 网卡已经是启用状态，无需再次启用
            }
        } else {
            if (up) {
                // 启用网卡
                ifr.ifr_flags |= IFF_UP;
                if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
                    spdlog::error("Failed to enable interface");
                    close(sockfd);
                    return false;
                }
            } else {
                // 网卡已经是禁用状态，无需再次禁用
            }
        }
        close(sockfd);
        return true;
    }

    static bool networkIsUp(const char *interface_name) {
        struct ifreq ifr{};
        int sockfd;
        // 打开一个套接字
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            spdlog::error("Failed to open socket");
            return false;
        }
        // 设置网卡名称
        strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
        // 获取网卡状态
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
            spdlog::error("Failed to get interface flags");
            close(sockfd);
            return false;
        }
        // 检查网卡状态
        bool up = ifr.ifr_flags & IFF_UP;
        close(sockfd);
        return up;
    }

    std::map<std::string, NetworkInfo> getNetworkInfos() {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
            spdlog::error("Failed to get network info");
            return {};
        }
        std::map<std::string, NetworkInfo> info_map;
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) {
                continue;
            }
            auto info_node = info_map.find(ifa->ifa_name);
            if (info_node == info_map.end()) {
                info_map.emplace(ifa->ifa_name, NetworkInfo{
                        .up = networkIsUp(ifa->ifa_name)
                });
                info_node = info_map.find(ifa->ifa_name);
            }
            family = ifa->ifa_addr->sa_family;
            if (family == AF_INET || family == AF_INET6) {
                s = getnameinfo(ifa->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST,
                                nullptr, 0, NI_NUMERICHOST);
                if (s != 0) {
                    spdlog::error("Failed to get network info: {}", gai_strerror(s));
                    return {};
                }
                info_node->second.name = ifa->ifa_name;
                if (family == AF_INET) {
                    info_node->second.inet = host;
                } else {
                    info_node->second.inet6 = host;
                }
            }
        }
        freeifaddrs(ifaddr);
        return info_map;
    }

    NetworkStats getNetworkStats(const std::string &interface) {
        NetworkStats stats = {0, 0};
        std::ifstream ifs("/proc/net/dev");
        if (!ifs.is_open()) {
            spdlog::error("Failed to open /proc/net/dev");
            return stats;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.find(interface) != std::string::npos) {
                std::istringstream iss(line);
                std::string iface;
                iss >> iface;
                iss >> stats.rxBytes >> stats.txBytes;
                break;
            }
        }
        return stats;
    }

    float getThermalZoneTemp() {
        std::ifstream ifs("/sys/class/thermal/thermal_zone0/temp");
        if (!ifs.is_open()) {
            spdlog::error("Failed to open thermal zone");
            return 0;
        }
        std::string line;
        if (!std::getline(ifs, line)) {
            spdlog::error("Failed to read thermal zone");
            ifs.close();
            return 0;
        }
        ifs.close();
        return static_cast<float>(std::stoi(line)) / 1000.0f;
    }

    MemoryInfo getMemoryInfo() {
        MemoryInfo info = {0, 0, 0, 0, 0};
        std::ifstream ifs("/proc/meminfo");
        if (!ifs.is_open()) {
            spdlog::error("Failed to open /proc/meminfo");
            return info;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            std::string key;
            unsigned long value;
            iss >> key >> value;
            value *= 1024;
            if (key == "MemTotal:") {
                info.total = value;
            } else if (key == "MemFree:") {
                info.free = value;
            } else if (key == "MemAvailable:") {
                info.available = value;
            } else if (key == "Buffers:") {
                info.buffers = value;
            } else if (key == "Cached:") {
                info.cached = value;
            } else if (key == "SwapTotal:") {
                info.swapTotal = value;
            } else if (key == "SwapFree:") {
                info.swapFree = value;
            }
        }
        return info;
    }

    DiskInfo getDiskInfo(const char *path) {
        DiskInfo info = {0, 0};
        struct statvfs stat{};
        if (statvfs(path, &stat) != 0) {
            spdlog::error("Failed to get disk info");
            return info;
        }
        info.total = stat.f_blocks * stat.f_bsize;
        info.free = stat.f_bfree * stat.f_bsize;
        return info;
    }

    static void getCpuInfo(unsigned long long &totalUser, unsigned long long &totalUserLow,
                           unsigned long long &totalSys, unsigned long long &totalIdle) {
        FILE *file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
               &totalSys, &totalIdle);
        fclose(file);
    }
    static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
    double getCpuRate() {
        unsigned long long totalUser;
        unsigned long long totalUserLow;
        unsigned long long totalSys;
        unsigned long long totalIdle;
        getCpuInfo(totalUser, totalUserLow, totalSys, totalIdle);
        double percent;
        unsigned long long total;
        if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
            totalSys < lastTotalSys || totalIdle < lastTotalIdle){
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else{
            total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                    (totalSys - lastTotalSys);
            percent = total;
            total += (totalIdle - lastTotalIdle);
            percent /= total;
            percent *= 100;
        }

        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;

        return percent;
    }

    static std::string SIZE_UNIT_NAME[]{
            "B",
            "K",
            "M",
            "G",
            "T",
    };

    std::string formatSize(float size) {
        int i = 0;
        while (size >= 1024 && i < 5) {
            size /= 1024;
            i++;
        }
        return fmt::format("{:.0f}{}", size, SIZE_UNIT_NAME[i]);
    }

    int64_t getUnixMill() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        // 将时间点转换为时间戳（自纪元以来的毫秒数）
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
}