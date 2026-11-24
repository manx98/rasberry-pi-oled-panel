#pragma once

#include "astra/ui/item/menu/menu.h"

namespace status_idle {

// Selectable item inside Settings: opens a compact two-button editor.
class SettingsEvent final : public astra::List::Event {
public:
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override {
        return true;
    }
};

std::string menuLabel();

} // namespace status_idle
