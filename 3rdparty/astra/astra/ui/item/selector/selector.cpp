//
// Created by Fir on 2024/4/14 014.
//

#include "astra/ui/item/selector/selector.h"

namespace astra {

    void Selector::setPosition() {
        //在go的时候改变trg的值
        if (menu->getType() == WIDGET_TYPE_TILE) {
            auto &childMenu = menu->childMenu[menu->selectIndex];
            xTrg = childMenu->position.xTrg - astraConfig.tileSelectBoxMargin;
            yTrg = childMenu->position.yTrg - astraConfig.tileSelectBoxMargin;

            yText = systemConfig.screenHeight; //给磁贴文字归零 从屏幕外滑入
            yTextTrg = systemConfig.screenHeight - childMenu->title.getHeight() - astraConfig.tileTextBottomMargin;
        } else if (menu->getType() == WIDGET_TYPE_LIST) {
            auto &childMenu = menu->childMenu[menu->selectIndex];
            xTrg = childMenu->position.xTrg - astraConfig.selectorMargin;
            yTrg = childMenu->position.yTrg;
        }
    }

/**
 * @brief
 *
 * @param _index
 * @note selector接管了移动选择指针的功能
 * @warning not support in loop. 不支持在循环内执行
 */
    void Selector::go(unsigned char _index) {
        Item::updateConfig();

//  if (_index > menu->childMenu.size() - 1) {
//    if (astraConfig.menuLoop) _index = 0;
//    else return;
//  } else if (_index < 0) {
//    if (astraConfig.menuLoop) _index = menu->childMenu.size() - 1;
//    else return;
//  }

        if (_index > menu->childMenu.size() - 1) return;
        if (_index < 0) return;
        menu->selectIndex = _index;

        setPosition();
    }

    void Selector::goNext() {
        if (this->menu->selectIndex == this->menu->childMenu.size() - 1) {
            if (astraConfig.menuLoop) go(0);
            else return;
        } else go(menu->selectIndex + 1);
        setPosition();
    }

    void Selector::goPreview() {
        if (this->menu->selectIndex == 0) {
            if (astraConfig.menuLoop) go(this->menu->childMenu.size() - 1);
            else return;
        } else go(menu->selectIndex - 1);
        setPosition();
    }

    bool Selector::inject(Menu *_menu) {
        if (_menu == nullptr) return false;

        this->menu = _menu;

        go(this->menu->selectIndex);  //注入之后要初始化选择框的位置

        return true;
    }

    bool Selector::destroy() {
        if (this->menu == nullptr) return false;

        delete this->menu;
        this->menu = nullptr;
        return true;
    }

    void Selector::render(std::vector<float> _camera, Clocker &clocker) {
        Item::updateConfig();

        //实际上 这里已经实现过渡动画了
        Animation::move(x, xTrg, astraConfig.selectorXAnimationSpeed, clocker);
        Animation::move(y, yTrg, astraConfig.selectorYAnimationSpeed, clocker);
        Animation::move(h, hTrg(), astraConfig.selectorHeightAnimationSpeed, clocker);
        Animation::move(w, wTrg(), astraConfig.selectorWidthAnimationSpeed, clocker);

        if (menu->getType() == WIDGET_TYPE_TILE) {
            Animation::move(yText, yTextTrg, astraConfig.selectorYAnimationSpeed, clocker);

            //draw m_text.
            //文字不受摄像机的影响
            HAL::setDrawType(1);
            auto &title = menu->childMenu[menu->selectIndex]->title;
            title.draw((systemConfig.screenWeight - title.getWidth()) / 2.0, yText + title.getHeight(), clocker);

            //draw box.
            //大框需要受摄像机的影响
            HAL::setDrawType(2);
            HAL::drawPixel(x + _camera[0], y + _camera[1]);
            //左上角
            HAL::drawHLine(x + _camera[0], y + _camera[1], astraConfig.tileSelectBoxLineLength + 1);
            HAL::drawVLine(x + _camera[0], y + _camera[1], astraConfig.tileSelectBoxLineLength + 1);
            //左下角
            HAL::drawHLine(x + _camera[0], y + _camera[1] + h - 1, astraConfig.tileSelectBoxLineLength + 1);
            HAL::drawVLine(x + _camera[0],
                           y + _camera[1] + h - astraConfig.tileSelectBoxLineLength - 1,
                           astraConfig.tileSelectBoxLineLength);
            //右上角
            HAL::drawHLine(x + _camera[0] + w - astraConfig.tileSelectBoxLineLength - 1,
                           y + _camera[1],
                           astraConfig.tileSelectBoxLineLength);
            HAL::drawVLine(x + _camera[0] + w - 1, y + _camera[1], astraConfig.tileSelectBoxLineLength + 1);
            //右下角
            HAL::drawHLine(x + _camera[0] + w - astraConfig.tileSelectBoxLineLength - 1,
                           y + _camera[1] + h - 1,
                           astraConfig.tileSelectBoxLineLength);
            HAL::drawVLine(x + _camera[0] + w - 1,
                           y + _camera[1] + h - astraConfig.tileSelectBoxLineLength - 1,
                           astraConfig.tileSelectBoxLineLength);

            HAL::drawPixel(x + _camera[0] + w - 1, y + _camera[1] + h - 1);
        } else if (menu->getType() == WIDGET_TYPE_LIST) {
            //draw select box.
            //受摄像机的影响
            HAL::setDrawType(2);
            HAL::drawRBox(x + _camera[0], y + _camera[1], w, h, astraConfig.selectorRadius);
            HAL::setDrawType(1);
        }
    }

    std::vector<float> Selector::getPosition() {
        return {xTrg, yTrg};
    }

    float Selector::hTrg() {
        if (menu->selectIndex < menu->childMenu.size()) {
            if (menu->getType() == WIDGET_TYPE_TILE) {
                return astraConfig.tileSelectBoxHeight;
            } else if (menu->getType() == WIDGET_TYPE_LIST) {
                auto &childMenu = menu->childMenu[menu->selectIndex];
                return childMenu->title.getHeight() + astraConfig.listTextMargin;
            }
        }
        return 0;
    }

    float Selector::wTrg() {
        if (menu->selectIndex < menu->childMenu.size()) {
            if (menu->getType() == WIDGET_TYPE_TILE) {
                auto &childMenu = menu->childMenu[menu->selectIndex];
                return astraConfig.tileSelectBoxWidth;
            } else if (menu->getType() == WIDGET_TYPE_LIST) {
                auto &childMenu = menu->childMenu[menu->selectIndex];
                return childMenu->getWidth();
            }
        }
        return 0;
    }
}