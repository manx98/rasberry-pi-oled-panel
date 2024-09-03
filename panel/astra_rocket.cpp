//
// Created by Fir on 2024/3/7 007.
//

//todo warning flash used age >= 90.93%, will be damaged soon

#include <vector>
#include "astra_rocket.h"
#include "astra_logo.h"
#include "sh1106_hal/sh1106_hal.h"
#include "icons/app_icons.h"

auto *astraLauncher = new astra::Launcher();
auto *rootPage = new astra::Tile("root");

bool test = false;
unsigned char testIndex = 0;
unsigned char testSlider = 60;

void astraCoreInit(void) {
  HAL::inject(new Sh1106Hal());

  HAL::delay(350);
  astra::drawLogo(5000);
  rootPage->addItem(new astra::List("系统状态", system_status_icon_30x30));
  rootPage->addItem(new astra::List("WIFI", wifi_icon_30x30));
  rootPage->addItem(new astra::List("Network", network_interface_icon_30x30));
  auto *secondPage = new astra::List("设置", setting_icon_30x30);
  rootPage->addItem(secondPage);

  secondPage->addItem(new astra::List());
  secondPage->addItem(new astra::List("-测试2"), new astra::CheckBox(test));
  secondPage->addItem(new astra::Tile("-测试测试3"), new astra::PopUp(1, "测试", {"测试"}, testIndex));
  secondPage->addItem(new astra::Tile("-测试测试测试4"), new astra::Slider("测试", 0, 100, 50, testSlider));
  secondPage->addItem(new astra::List("-测试测试测试5"));
  secondPage->addItem(new astra::List("-测试测试测试6"));
  secondPage->addItem(new astra::List("-测试测试测试6"));
  secondPage->addItem(new astra::List("-测试测试测试6"));
  secondPage->addItem(new astra::List("-测试测试测试6"));

  astraLauncher->init(rootPage);
}

void astraCoreStart(void) {
  for (;;) {  //NOLINT
      astraLauncher->update();
//    astraCoreTest();
  }
}

void astraCoreDestroy(void) {
  HAL::destroy();
  delete astraLauncher;
}
