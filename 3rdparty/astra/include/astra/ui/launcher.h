//
// Created by Fir on 2024/2/2.
//
#pragma once
#ifndef ASTRA_CORE_SRC_ASTRA_UI_SCHEDULER_H_
#define ASTRA_CORE_SRC_ASTRA_UI_SCHEDULER_H_

#include "item/menu/menu.h"
#include "item/selector/selector.h"
#include "item/camera/camera.h"

namespace astra {
    class Launcher {
    private:
        static Menu *currentMenu;
        static Widget *currentWidget;
        static Selector *selector;
        static Camera *camera;
    public:
        static void popInfo(std::string _info, uint16_t _time);

        static void init(Menu *_rootPage);

        static bool open();

        static bool close();

        static void update(const std::function<void(Clocker &clocker, key::keyIndex active_key)>& render);

        static void update();

        static key::keyIndex getKey();

        static Camera *getCamera() { return camera; }

        static Selector *getSelector() { return selector; }
    };
}

#endif //ASTRA_CORE_SRC_ASTRA_UI_SCHEDULER_H_