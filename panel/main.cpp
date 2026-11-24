#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <csignal>
#include <exception>
#include "utils/shutdown.h"
#include "utils/wifi.h"

#include "astra_rocket.h"
#include "sh1106_hal/sdl_hal.h"
#include "sh1106_hal/sh1106_hal.h"

namespace {
void handleStopSignal(int) noexcept { panel_shutdown::request(); }

bool installStopHandlers() {
    struct sigaction action{};
    action.sa_handler = handleStopSignal;
    sigemptyset(&action.sa_mask);
    return sigaction(SIGTERM, &action, nullptr) == 0 &&
           sigaction(SIGINT, &action, nullptr) == 0;
}
}

int main(int argc, char **argv) {
    if (argc == 2 && std::strcmp(argv[1], "--clear-display") == 0) {
#ifdef USE_SDL2
        return 0;
#else
        return clearSh1106Display();
#endif
    }
    // Headless smoke test for the NetworkManager scan path. No GPIO or OLED
    // is claimed, so it can run while the panel is already active.
    if (argc >= 2 && std::strcmp(argv[1], "--test-wifi-scan") == 0) {
        const char *iface = argc >= 3 ? argv[2] : "wlan0";
        int repeat = argc >= 4 ? std::atoi(argv[3]) : 1;
        if (repeat < 1 || repeat > 20) {
            std::fprintf(stderr, "repeat must be in the range 1..20\n");
            return 2;
        }
        for (int n = 0; n < repeat; ++n) {
            const auto start = std::chrono::steady_clock::now();
            const auto networks = RPI::getWifiList(iface);
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            std::printf("scan %d/%d: %zu APs, %lld ms\n",
                        n + 1, repeat, networks.size(), static_cast<long long>(elapsed));
            std::fflush(stdout);
        }
        return 0;
    }
#ifndef USE_SDL2
    if (geteuid() != 0) {
        std::fprintf(stderr, "OLED panel requires root. Start with systemctl start raspberry-pi-oled-panel.service or sudo.\n");
        return 1;
    }
#endif
    if (!installStopHandlers()) {
        std::perror("install shutdown signal handlers");
        return 1;
    }
    int status = 0;
    try {
        astraCoreInit();
        astraCoreStart();
    } catch (const panel_shutdown::Requested &) {
        // Normal cancellation from any nested UI frame.
    } catch (const std::exception &e) {
        std::fprintf(stderr, "OLED panel failed: %s\n", e.what());
        status = 1;
    } catch (...) {
        std::fprintf(stderr, "OLED panel failed: unknown exception\n");
        status = 1;
    }
    panel_shutdown::request();
    if (astraCoreDestroy() != 0) status = 1;
    return status;
}
