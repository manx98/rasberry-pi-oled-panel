#ifndef RASPBERRY_PI_OLED_PANEL_UTILS_H
#define RASPBERRY_PI_OLED_PANEL_UTILS_H

#include <list>
#include <string>
#include <map>

struct WifiInfo {
    std::string BSS;
    std::string SSID;
    float signal;
};

struct WifiConnectInfo {
    std::string BSS;
    std::string SSID;
    int64_t RX;
    int64_t TX;
    float signal;
};

WifiConnectInfo *getWifiConnectInfo(const std::string &interface_name);

std::list<WifiInfo> getWifiList(const std::string &interface_name);

std::list<std::string> getWifiDevices();

bool setIpLinkUp(const char* interface_name, bool up);

struct NetworkInfo {
    std::string name;
    std::string inet;
    std::string inet6;
    bool up;
};

std::map<std::string, NetworkInfo> getNetworkInfos();

struct NetworkStats {
    unsigned long long rxBytes{};
    unsigned long long txBytes{};
};

NetworkStats getNetworkStats(const std::string& interface);

int getThermalZoneTemp();

struct MemoryInfo {
    unsigned long total;
    unsigned long free;
    unsigned long available;
    unsigned long buffers;
    unsigned long cached;
    unsigned long swapTotal;
    unsigned long swapFree;
};
MemoryInfo getMemoryInfo();

struct DiskInfo {
    unsigned long long total;
    unsigned long long free;
};
DiskInfo getDiskInfo(const char* path);
#endif //RASPBERRY_PI_OLED_PANEL_UTILS_H
