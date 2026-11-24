#include "utils/wifi_nmcli.h"
#include "utils/shutdown.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <cctype>
#include <cstring>
#include <spawn.h>
#include <string>
#include <sys/socket.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

extern char **environ;

#ifndef RPI_NMCLI_BINARY
#define RPI_NMCLI_BINARY "/usr/bin/nmcli"
#endif

namespace RPI {
namespace {

// nmcli --ask is interactive by design. A pseudo-terminal provides the
// expected terminal semantics (including prompts/readline), unlike an input
// pipe that some versions of nmcli simply refuse to read from.
WifiCommandResult runNmcliInteractive(const std::vector<std::string> &args,
                                      const std::string &password) {
    int master = ::posix_openpt(O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (master < 0) return WifiCommandResult::Unavailable;
    char terminalPath[128]{};
    if (::grantpt(master) != 0 || ::unlockpt(master) != 0
        || ::ptsname_r(master, terminalPath, sizeof(terminalPath)) != 0) {
        ::close(master);
        return WifiCommandResult::Unavailable;
    }
    const int slave = ::open(terminalPath, O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (slave < 0) { ::close(master); return WifiCommandResult::Unavailable; }

    // Construct argv and a fixed English locale *before* fork. After forking
    // the multithreaded panel, the child uses only async-signal-safe syscalls.
    std::vector<char *> argv;
    argv.reserve(args.size() + 2);
    argv.push_back(const_cast<char *>("nmcli"));
    for (const auto &a : args) argv.push_back(const_cast<char *>(a.c_str()));
    argv.push_back(nullptr);
    std::vector<char *> envp;
    for (char **p = environ; p && *p; ++p) {
        if (std::strncmp(*p, "LC_ALL=", 7) != 0) envp.push_back(*p);
    }
    envp.push_back(const_cast<char *>("LC_ALL=C"));
    envp.push_back(nullptr);

    const pid_t pid = ::fork();
    if (pid < 0) { ::close(master); ::close(slave); return WifiCommandResult::Unavailable; }
    if (pid == 0) {
        if (::setsid() < 0 || ::ioctl(slave, TIOCSCTTY, 0) < 0) ::_exit(127);
        if (::dup2(slave, STDIN_FILENO) < 0 || ::dup2(slave, STDOUT_FILENO) < 0
            || ::dup2(slave, STDERR_FILENO) < 0) ::_exit(127);
        ::close(master);
        ::close(slave);
        ::execve(RPI_NMCLI_BINARY, argv.data(), envp.data());
        ::_exit(127);
    }
    ::close(slave);
    const int flags = ::fcntl(master, F_GETFL, 0);
    if (flags >= 0) ::fcntl(master, F_SETFL, flags | O_NONBLOCK);

    bool providedSecret = false;
    std::string prompt;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(38);
    int status = 0;
    bool timedOut = true;
    for (;;) {
        const pid_t finished = ::waitpid(pid, &status, WNOHANG);
        if (finished == pid) { timedOut = false; break; }
        if (finished < 0 && errno != EINTR) { timedOut = false; break; }
        if (panel_shutdown::stopping() || std::chrono::steady_clock::now() >= deadline) break;

        pollfd pfd{master, static_cast<short>(POLLIN | POLLHUP), 0};
        if (::poll(&pfd, 1, 100) <= 0 || !(pfd.revents & POLLIN)) continue;
        char chunk[512];
        const ssize_t n = ::read(master, chunk, sizeof(chunk));
        if (n <= 0) continue;
        if (providedSecret) continue; // Never retain any echoed password.
        for (ssize_t i = 0; i < n; ++i) {
            prompt.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(chunk[i]))));
        }
        if (prompt.size() > 1024) prompt.erase(0, prompt.size() - 1024);
        if (prompt.find("password") != std::string::npos
            || prompt.find("passphrase") != std::string::npos
            || prompt.find("wep key") != std::string::npos) {
            std::string entry = password + '\n';
            std::size_t offset = 0;
            while (offset < entry.size() && !panel_shutdown::stopping() &&
                   std::chrono::steady_clock::now() < deadline) {
                const auto written = ::write(master, entry.data() + offset, entry.size() - offset);
                if (written > 0) offset += static_cast<std::size_t>(written);
                else if (written < 0 && errno == EINTR) continue;
                else if (written < 0 && errno == EAGAIN) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } else break;
            }
            volatile char *bytes = entry.empty() ? nullptr : &entry[0];
            for (std::size_t i = 0; bytes && i < entry.size(); ++i) bytes[i] = 0;
            providedSecret = true;
            prompt.clear();
        }
    }
    ::close(master);
    if (timedOut) {
        ::kill(pid, SIGTERM);
        for (int i = 0; i < 20; ++i) {
            const auto finished = ::waitpid(pid, &status, WNOHANG);
            if (finished == pid || (finished < 0 && errno == ECHILD))
                return WifiCommandResult::Timeout;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ::kill(pid, SIGKILL);
        while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
        return WifiCommandResult::Timeout;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? WifiCommandResult::Success : WifiCommandResult::Failed;
}

WifiCommandResult runNmcli(const std::vector<std::string> &args, const std::string *password) {
    if (args.empty()) return WifiCommandResult::Unavailable;
    if (panel_shutdown::stopping()) return WifiCommandResult::Timeout;
    if (password) return runNmcliInteractive(args, *password);
    int input[2] = {-1, -1};
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, input) != 0) {
        return WifiCommandResult::Unavailable;
    }
    const int nullFd = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (nullFd < 0) {
        ::close(input[0]);
        ::close(input[1]);
        return WifiCommandResult::Unavailable;
    }

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        ::close(input[0]); ::close(input[1]); ::close(nullFd);
        return WifiCommandResult::Unavailable;
    }
    posix_spawn_file_actions_adddup2(&actions, input[1], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, nullFd, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, nullFd, STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, input[0]);
    posix_spawn_file_actions_addclose(&actions, input[1]);
    posix_spawn_file_actions_addclose(&actions, nullFd);

    std::vector<char *> argv;
    argv.reserve(args.size() + 2);
    argv.push_back(const_cast<char *>("nmcli"));
    for (const auto &argument : args) argv.push_back(const_cast<char *>(argument.c_str()));
    argv.push_back(nullptr);
    pid_t pid = -1;
    const int spawnError = ::posix_spawn(&pid, RPI_NMCLI_BINARY, &actions, nullptr,
                                         argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    ::close(input[1]);
    ::close(nullFd);
    if (spawnError != 0) {
        ::close(input[0]);
        return WifiCommandResult::Unavailable;
    }

    ::shutdown(input[0], SHUT_WR);
    ::close(input[0]);

    // nmcli itself is limited to 30s. A separate watchdog prevents the UI
    // worker from waiting indefinitely if the process or D-Bus stalls.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(38);
    int status = 0;
    for (;;) {
        const pid_t finished = ::waitpid(pid, &status, WNOHANG);
        if (finished == pid) {
            return WIFEXITED(status) && WEXITSTATUS(status) == 0
                       ? WifiCommandResult::Success : WifiCommandResult::Failed;
        }
        if (finished < 0 && errno != EINTR) return WifiCommandResult::Failed;
        if (panel_shutdown::stopping() || std::chrono::steady_clock::now() >= deadline) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ::kill(pid, SIGTERM);
    for (int i = 0; i < 20; ++i) {
        const pid_t finished = ::waitpid(pid, &status, WNOHANG);
        if (finished == pid || (finished < 0 && errno == ECHILD)) return WifiCommandResult::Timeout;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ::kill(pid, SIGKILL);
    while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
    return WifiCommandResult::Timeout;
}

} // namespace

WifiCommandResult wifiConnectNew(const std::string &iface, const std::string &ssid,
                                 const std::string &bssid, const std::string &password) {
    if (iface.empty() || ssid.empty()) return WifiCommandResult::Failed;
    std::vector<std::string> args;
    if (!password.empty()) args.push_back("--ask");
    args.insert(args.end(), {"--wait", "30", "device", "wifi", "connect", ssid, "ifname", iface});
    if (!bssid.empty()) args.insert(args.end(), {"bssid", bssid});
    return runNmcli(args, password.empty() ? nullptr : &password);
}

WifiCommandResult wifiConnectSaved(const std::string &iface, const std::string &uuid) {
    if (iface.empty() || uuid.empty()) return WifiCommandResult::Failed;
    return runNmcli({"--wait", "30", "connection", "up", "uuid", uuid, "ifname", iface}, nullptr);
}

WifiCommandResult wifiDisconnect(const std::string &iface) {
    if (iface.empty()) return WifiCommandResult::Failed;
    return runNmcli({"--wait", "30", "device", "disconnect", iface}, nullptr);
}

WifiCommandResult wifiForgetSaved(const std::string &connectionUuid) {
    if (connectionUuid.empty()) return WifiCommandResult::Failed;
    return runNmcli({"--wait", "30", "connection", "delete", "uuid", connectionUuid}, nullptr);
}

} // namespace RPI
