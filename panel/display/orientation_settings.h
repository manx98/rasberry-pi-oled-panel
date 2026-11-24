#pragma once
#include "astra/ui/item/menu/menu.h"

namespace display {
class OrientationSettingsEvent final : public astra::List::Event {
public:
    bool beforeOpen(astra::Menu *current) override;
    bool beforeRender(astra::Menu *, const std::vector<float> &, astra::Clocker &) override { return true; }
};
}
