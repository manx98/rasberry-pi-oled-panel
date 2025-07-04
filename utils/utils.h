#ifndef RASPBERRY_PI_OLED_PANEL_UTILS_UTILS_H
#define RASPBERRY_PI_OLED_PANEL_UTILS_UTILS_H

#include <list>
#include <string>
#include <map>
#include "wifi.h"

namespace RPI {
    bool setIpLinkUp(const char* interface_name, bool up);

    struct NetworkInfo {
        std::string name;
        std::string inet;
        std::string inet6;
        bool up;
    };

    std::map<std::string, NetworkInfo> getNetworkInfos();

    struct NetworkStats {
        int64_t rxBytes{};
        int64_t txBytes{};
    };

    NetworkStats getNetworkStats(const std::string& interface);

    float getThermalZoneTemp();

    struct MemoryInfo {
        int64_t total;
        int64_t free;
        int64_t available;
        int64_t buffers;
        int64_t cached;
        int64_t swapTotal;
        int64_t swapFree;
    };
    MemoryInfo getMemoryInfo();

    struct DiskInfo {
        int64_t total;
        int64_t free;
    };
    DiskInfo getDiskInfo(const char* path);

    double getCpuRate();

    std::string formatSize(float size);

    int64_t getUnixMill();
}
#endif //RASPBERRY_PI_OLED_PANEL_UTILS_UTILS_H
