//
// Created by Fir on 2024/2/2.
//

#include "astra/ui/launcher.h"
#include "astra/utils/clocker.h"

namespace astra {
    void Launcher::popInfo(std::string _info, uint16_t _time) {
        float wPop = HAL::getFontWidth(_info) + 2 * getUIConfig().popMargin;  //宽度
        float hPop = HAL::getFontHeight() + 2 * getUIConfig().popMargin;  //高度
        float yPop = 0 - hPop - 8; //从屏幕上方滑入
        float yPopTrg = (HAL::getSystemConfig().screenHeight - hPop) / 3;  //目标位置 中间偏上
        float xPop = (HAL::getSystemConfig().screenWeight - wPop) / 2;  //居中
        Clocker render_clocker;
        Clocker pop_clocker;
        while (true) {
            render_clocker.waitUntil(getUIConfig().perFrameMills);
            render_clocker.next();
            HAL::canvasClear();
            /*渲染一帧*/
            currentMenu->render(camera->getPosition(), render_clocker);
            selector->render(camera->getPosition(), render_clocker);
            camera->update(currentMenu, selector, render_clocker);
            /*渲染一帧*/

            HAL::setDrawType(0);
            HAL::drawRBox(xPop - 4, yPop - 4, wPop + 8, hPop + 8, getUIConfig().popRadius + 2);
            HAL::setDrawType(1);  //反色显示
            HAL::drawRFrame(xPop - 1, yPop - 1, wPop + 2, hPop + 2, getUIConfig().popRadius);  //绘制一个圆角矩形
            HAL::drawEnglish(xPop + getUIConfig().popMargin,
                             yPop + getUIConfig().popMargin + HAL::getFontHeight(),
                             _info);  //绘制文字

            Animation::move(yPop, yPopTrg, getUIConfig().popSpeed, render_clocker);  //动画
            HAL::canvasUpdate();

            //这里条件可以加上一个如果按键按下 就滑出
            if (pop_clocker.currentDuration() >= _time || HAL::keyScan()) {
                yPopTrg = 0 - hPop - 8; //滑出
            }

            if (yPop == 0 - hPop - 8) {
                return;  //退出条件
            }
        }
    }

    void Launcher::init(Menu *_rootPage) {
        currentMenu = _rootPage;

        camera = new Camera(0, 0);
        _rootPage->childPosInit(camera->getPosition());

        selector = new Selector();
        selector->inject(_rootPage);

        camera->init(_rootPage->getType());
    }

/**
 * @brief 打开选中的页面
 *
 * @return 是否成功打开
 * @warning 仅可调用一次
 */
    bool Launcher::open() {

        //如果当前页面指向的当前item没有后继 那就返回false
        if (currentMenu->getNextMenu() == nullptr) {
            popInfo("unreferenced page!", 800);
            return false;
        }
        if (currentMenu->getNextMenu()->getItemNum() == 0) {
            popInfo("empty page!", 800);
            return false;
        }

        currentMenu->rememberCameraPos(camera->getPositionTrg());

        currentMenu->deInit();  //先析构（退场动画）再挪动指针

        currentMenu = currentMenu->getNextMenu();
        currentMenu->forePosInit();
        currentMenu->childPosInit(camera->getPosition());

        selector->inject(currentMenu);
        return true;
    }

    /**
     * @brief 关闭选中的页面
     *
     * @return 是否成功关闭
     * @warning 仅可调用一次
     */
    bool Launcher::close() {
        if (currentMenu->getPreview() == nullptr) {
            popInfo("unreferenced page!", 600);
            return false;
        }
        if (currentMenu->getPreview()->getItemNum() == 0) {
            popInfo("empty page!", 600);
            return false;
        }

        currentMenu->rememberCameraPos(camera->getPositionTrg());

        currentMenu->deInit();  //先析构（退场动画）再挪动指针

        currentMenu = currentMenu->getPreview();
        currentMenu->forePosInit();
        currentMenu->childPosInit(camera->getPosition());

        selector->inject(currentMenu);
        //selector->go(currentPage->selectIndex);

        return true;
    }

    void Launcher::update() {
        static Clocker render_clocker;
        static Clocker key_clocker;
        render_clocker.waitUntil(getUIConfig().perFrameMills);
        render_clocker.next();
        HAL::canvasClear();
        currentMenu->render(camera->getPosition(), render_clocker);
        if (currentWidget != nullptr) currentWidget->render(camera->getPosition());
        selector->render(camera->getPosition(), render_clocker);
        camera->update(currentMenu, selector, render_clocker);
        if (key_clocker.currentDuration() > 50) {
            if (HAL::keyScan()) {
                if (HAL::getKeyMap()[key::KEY_PREV] == key::CLICK) {
                    selector->goPreview(); // selector去到上一个项目
                } else if (HAL::getKeyMap()[key::KEY_NEXT] == key::CLICK) {
                    selector->goNext(); //selector去到下一个项目
                } else if (HAL::getKeyMap()[key::KEY_CONFIRM] == key::CLICK) {
                    open(); //打开当前项目
                } else if (HAL::getKeyMap()[key::KEY_CANCEL] == key::CLICK) {
                    close(); //退出当前项目
                }
            }
            key_clocker.reset();
        }
        HAL::canvasUpdate();
    }
}