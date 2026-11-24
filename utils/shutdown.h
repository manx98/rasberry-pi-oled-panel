#pragma once

#include <atomic>
#include <thread>

namespace panel_shutdown {
// Only a lock-free atomic store is performed in the POSIX signal handler.
// The flag is also read by the background network workers.
inline std::atomic<bool> requested{false};
static_assert(std::atomic<bool>::is_always_lock_free);
inline void request() noexcept { requested.store(true, std::memory_order_relaxed); }
inline bool stopping() noexcept { return requested.load(std::memory_order_relaxed); }

struct Requested {};
inline void checkpoint() { if (stopping()) throw Requested{}; }

// UI cancellation can unwind nested modal calls. Join workers before their
// stack-captured data goes away, rather than destroying a joinable thread.
class JoinThread {
public:
    explicit JoinThread(std::thread &thread) noexcept : thread_(thread) {}
    ~JoinThread() { if (thread_.joinable()) thread_.join(); }
    JoinThread(const JoinThread &) = delete;
    JoinThread &operator=(const JoinThread &) = delete;
private:
    std::thread &thread_;
};
} // namespace panel_shutdown
