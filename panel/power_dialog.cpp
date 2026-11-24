#include "panel/power_dialog.h"
#include "panel/power_dialog_state.h"
#include "astra/ui/launcher.h"
#include "astra/hal/hal.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

namespace {

void drawRoot(astra::Clocker &clocker) {
    auto *menu = astra::Launcher::getCurrentMenu();
    auto *camera = astra::Launcher::getCamera();
    auto *selector = astra::Launcher::getSelector();
    menu->doRender(camera->getPosition(), clocker);
    selector->render(camera->getPosition(), clocker);
    camera->update(menu, selector, clocker);
}

void drawDialog(const PowerDialogState &dialog) {
    auto *font = astra::getUIConfig().mainFont;
    HAL::setDrawType(0);
    HAL::drawRBox(4, 3, 120, 58, 3);
    HAL::setDrawType(1);
    HAL::drawRFrame(4, 3, 120, 58, 3);
    HAL::setFont(font);
    if (dialog.page() == PowerDialogState::Page::Menu) {
        HAL::drawChinese(13, 16, "电源管理");
        HAL::drawHLine(11, 19, 106);
        HAL::drawChinese(16, 36, "关机");
        HAL::drawChinese(16, 49, "重启");
        HAL::setDrawType(2); // Highlight using XOR: text stays legible.
        HAL::drawRBox(11, 26 + dialog.selected() * 13, 106, 12, 1);
    } else {
        const char *title = dialog.requestedAction() == PowerDialogState::Action::PowerOff
                                ? "确定关机?" : "确定重启?";
        HAL::drawChinese(13, 17, title);
        HAL::drawHLine(11, 21, 106);
        HAL::drawChinese(16, 36, "执行");
        HAL::drawChinese(16, 49, "返回");
        HAL::setDrawType(2);
        HAL::drawRBox(11, 26 + dialog.selected() * 13, 106, 12, 1);
    }
    HAL::setDrawType(1);
}

void drawMessage(const char *message) {
    HAL::canvasClear();
    HAL::setDrawType(1);
    HAL::setFont(astra::getUIConfig().mainFont);
    HAL::drawRFrame(4, 15, 120, 34, 3);
    HAL::drawChinese(14, 36, message);
}

// The panel is started as root by scripts/run-panel-root.sh.
// Invoke systemctl directly with a fixed argument vector and no shell.
int requestPower(PowerDialogState::Action action) {
    const char *verb = action == PowerDialogState::Action::PowerOff ? "poweroff" : "reboot";
    const pid_t child = fork();
    if (child < 0) {
        std::perror("fork power action");
        return -1;
    }
    if (child == 0) {
        execl("/usr/bin/systemctl", "systemctl", "--no-block", verb,
              static_cast<char *>(nullptr));
        _exit(127);
    }
    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) return -1;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
}

} // namespace

void showPowerManagementDialog() {
    PowerDialogState dialog;
    do {
        astra::Launcher::update(true, [&](astra::Clocker &clocker, key::keyIndex key) {
            drawRoot(clocker);
            switch (key) {
                case key::KEY_PREV: dialog.press(PowerDialogState::Key::Previous); break;
                case key::KEY_NEXT: dialog.press(PowerDialogState::Key::Next); break;
                case key::KEY_CONFIRM: dialog.press(PowerDialogState::Key::Confirm); break;
                case key::KEY_CANCEL: dialog.press(PowerDialogState::Key::Cancel); break;
                default: break;
            }
            if (!dialog.finished()) drawDialog(dialog);
        });
    } while (!dialog.finished());

    if (dialog.confirmedAction() == PowerDialogState::Action::None) return;
    astra::Launcher::update(true, [&](astra::Clocker &, key::keyIndex) {
        drawMessage(dialog.confirmedAction() == PowerDialogState::Action::PowerOff
                        ? "正在关机..." : "正在重启...");
    });
    if (requestPower(dialog.confirmedAction()) != 0) {
        std::fprintf(stderr, "Power command failed; check root launch and systemctl logs.\n");
        astra::Launcher::popInfo({0, {{"电源操作失败", astra::getUIConfig().mainFont}}}, 1800);
    }
}
