//
// Created by Fir on 2024/3/7 007.
//
#include <vector>
#include "wifi.h"
#include "astra_rocket.h"
#include "astra_logo.h"
#include "sh1106_hal/sh1106_hal.h"
#include "icons/app_icons.h"
#include "text.h"
#include "system.h"
#include "network.h"

bool test = false;
unsigned char testIndex = 0;
unsigned char testSlider = 60;


void astraCoreInit(void) {
    HAL::inject(new Sh1106Hal());

    HAL::delay(350);
//  astra::drawLogo(5000);
    auto font = astra::getUIConfig().mainFont;
    auto textMarin = astra::getUIConfig().listTextMargin;
    auto *rootPage = new astra::Tile({textMarin, {{"root", font}}});
    rootPage->addItem(
            new astra::List({textMarin, {{SYSTEM_STATUS_TEXT, font}}}, system_status_icon_30x30,
                            new SystemStatusListEvent()));
    rootPage->addItem(new astra::List({textMarin, {{WIFI_STATUS_TEXT, font}}}, wifi_icon_30x30, new WifiDeviceEvent()));
    rootPage->addItem(new astra::List({textMarin, {{NETWORK_INTERFACE_TEXT, font}}}, network_interface_icon_30x30,
                                      new NetWorkDeviceListEvent()));
    auto *secondPage = new astra::List({textMarin, {{SETTING_TEXT, font}}}, setting_icon_30x30);
    rootPage->addItem(secondPage);

    secondPage->addItem(new astra::List({textMarin, {{"-测试2", font}}}), new astra::CheckBox(test, [](bool &checked) {
        printf("checked --------> %d\n", checked);
    }));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试4", font}}}),
                        new astra::Slider("测试", 0, 100, 50, testSlider));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试5", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {
            {"-111111", font},
            {" 222222", font},
    }}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));
    secondPage->addItem(new astra::List({textMarin, {{"-测试测试测试6", font}}}));

    astra::Launcher::init(rootPage);
}

[[noreturn]] void astraCoreStart(void) {
    for (;;) {  //NOLINT
        astra::Launcher::update();
    }
}

void astraCoreDestroy(void) {
    HAL::destroy();
}
