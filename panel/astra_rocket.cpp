//
// Created by Fir on 2024/3/7 007.
//
#include <vector>
#include "wifi.h"
#include "astra_rocket.h"
#include "astra_logo.h"
#include "icons/app_icons.h"
#include "text.h"
#include "system.h"
#include "network.h"
#include "panel/power_dialog.h"
#include "panel/status_idle_config.h"
#include "panel/status_idle_settings.h"
#include "panel/display/orientation_config.h"
#include "panel/display/orientation_settings.h"
#if defined(PANEL_ENABLE_FAN) && !defined(USE_SDL2)
#include "panel/fan/fan_controller.h"
#include "panel/fan/fan_settings.h"
#endif
#ifdef PANEL_ENABLE_SENSORS
#include "common/sensor.h"
#endif
#ifdef USE_SDL2
#include "sh1106_hal/sdl_hal.h"
#else
#include "sh1106_hal/sh1106_hal.h"
#endif

bool test = false;
unsigned char testIndex = 0;
unsigned char testSlider = 60;


void astraCoreInit(void) {
    status_idle::Config::instance().load();
    // Orientation must be loaded before SH1106 initialization and input.
    display::OrientationConfig::instance().load();
#if defined(PANEL_ENABLE_FAN) && !defined(USE_SDL2)
    // Starts autonomous GPIO12 control independently of OLED refresh or UI.
    fan::Controller::instance().start();
#endif
#ifdef PANEL_ENABLE_SENSORS
    if (init_sensor() != 0) {
        fprintf(stderr, "Optional sensor initialization failed; continuing with OLED UI.\n");
    }
#endif
#ifdef USE_SDL2
    HAL::inject(new SDLHal());
#else
    HAL::inject(new Sh1106Hal());
#endif

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
    secondPage->addItem(new astra::List(
        {textMarin, {{status_idle::menuLabel(), font}}}, new status_idle::SettingsEvent()));
    secondPage->addItem(new astra::List(
        {textMarin, {{"屏幕方向", font}}}, new display::OrientationSettingsEvent()));

#if defined(PANEL_ENABLE_FAN) && !defined(USE_SDL2)
    secondPage->addItem(new astra::List({textMarin, {{"风扇温控", font}}}, new fan::SettingsEvent()));
#endif

    astra::Launcher::init(rootPage);
    astra::Launcher::setRootCancelHandler(showPowerManagementDialog);
}

[[noreturn]] void astraCoreStart(void) {
    for (;;) {  //NOLINT
        astra::Launcher::update();
    }
}

void astraCoreDestroy(void) {
    HAL::destroy();
}
