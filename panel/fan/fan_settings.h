#pragma once

#include "astra/ui/item/menu/menu.h"

namespace fan {
// Menu content is populated when opened to show latest persisted config.
class SettingsEvent final : public astra::List::Event {
public:
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *current, const std::vector<float> &camera,
                      astra::Clocker &clocker) override;
private:
    long long lastStatusRefreshMs_ = 0;
};
}
