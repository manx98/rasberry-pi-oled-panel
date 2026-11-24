#include "panel/status_idle_config.h"
#include <cassert>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>

int main() {
    assert(status_idle::validSeconds(0));
    assert(status_idle::validSeconds(5));
    assert(status_idle::validSeconds(30));
    assert(status_idle::validSeconds(300));
    assert(!status_idle::validSeconds(-5));
    assert(!status_idle::validSeconds(31));
    assert(!status_idle::validSeconds(305));
    char temp[] = "/tmp/oled-status-idle.XXXXXX";
    const char *dir = mkdtemp(temp);
    assert(dir != nullptr);
    const std::string path = std::string(dir) + "/status-idle.conf";
    status_idle::Config initial(path);
    initial.load();
    assert(initial.seconds() == 30);
    assert(!initial.setSeconds(31));
    assert(initial.seconds() == 30);
    assert(initial.setSeconds(0));
    assert(initial.seconds() == 0);
    status_idle::Config reload(path);
    reload.load();
    assert(reload.seconds() == 0);
    assert(reload.setSeconds(120));
    status_idle::Config last(path);
    last.load();
    assert(last.seconds() == 120);
    { std::ofstream f(path); f << "999\n"; }
    status_idle::Config invalid(path);
    invalid.load();
    assert(invalid.seconds() == 30);
    unlink(path.c_str());
    rmdir(dir);
}
