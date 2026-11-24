#include "panel/display/orientation_settings.h"
#include "panel/display/orientation_config.h"
#include "astra/ui/launcher.h"

namespace display {
namespace {
std::string optionLabel(int degrees) {
    return std::string(degrees == 0 ? "0度" : "180度") +
           (OrientationConfig::instance().degrees() == degrees ? " [当前]" : "");
}

class ChoiceEvent final : public astra::List::Event {
public:
    explicit ChoiceEvent(int degrees) : degrees_(degrees) {}

    bool beforeOpen(astra::Menu *current) override {
        if (!OrientationConfig::instance().setDegrees(degrees_)) {
            astra::Launcher::popInfo({0, {{"保存屏幕方向失败", astra::getUIConfig().mainFont}}}, 1300);
            return false;
        }
        if (auto *parent = current->getPreview(); parent && parent->childMenu.size() == 2) {
            for (int index = 0; index < 2; ++index) {
                parent->childMenu[index]->title.setText(
                    0, {optionLabel(index == 0 ? 0 : 180), astra::getUIConfig().mainFont});
            }
        }
        return false; // Change in place; remain on the orientation choices.
    }
    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override { return true; }
private:
    int degrees_;
};
}

bool OrientationSettingsEvent::beforeOpen(astra::Menu *current) {
    current->clear();
    current->selectIndex = OrientationConfig::instance().degrees() == 180 ? 1 : 0;
    current->resetCameraMemoryPos();
    const auto margin = astra::getUIConfig().listTextMargin;
    const auto font = astra::getUIConfig().mainFont;
    current->addItem(new astra::List({margin, {{optionLabel(0), font}}}, new ChoiceEvent(0)));
    current->addItem(new astra::List({margin, {{optionLabel(180), font}}}, new ChoiceEvent(180)));
    return true;
}
}
