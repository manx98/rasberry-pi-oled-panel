#include "utils/shutdown.h"
#include <cassert>
#include <chrono>
#include <csignal>

namespace {
void onSignal(int) { panel_shutdown::request(); }
}

int main(int argc, char **) {
    const int signal = argc > 1 ? SIGINT : SIGTERM;
    struct sigaction action{};
    action.sa_handler = onSignal;
    sigemptyset(&action.sa_mask);
    assert(sigaction(signal, &action, nullptr) == 0);
    assert(!panel_shutdown::stopping());
    bool joined = false;
    bool unwound = false;
    try {
        std::thread worker([&] {
            while (!panel_shutdown::stopping()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            joined = true;
        });
        panel_shutdown::JoinThread guard(worker);
        const auto nestedModal = [&] {
            assert(raise(signal) == 0);
            panel_shutdown::checkpoint();
            assert(false);
        };
        nestedModal();
    } catch (const panel_shutdown::Requested &) {
        unwound = true;
    }
    assert(joined && unwound && panel_shutdown::stopping());
}
