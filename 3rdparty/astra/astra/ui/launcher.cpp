//
// Created by Fir on 2024/2/2.
//

#include "astra/ui/launcher.h"
#include "astra/utils/clocker.h"

namespace astra {
    Menu *Launcher::currentMenu = {};
    Widget *Launcher::currentWidget = {};
    Selector *Launcher::selector = {};
    Camera *Launcher::camera = {};

    void Launcher::popInfo(const TextBox &info, uint16_t _time) {
        float wPop = info.getWidth() + 2 * getUIConfig().popMargin;  //宽度
        float hPop = info.getHeight() + 2 * getUIConfig().popMargin;  //高度
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
            info.draw(xPop + getUIConfig().popMargin, yPop + getUIConfig().popMargin + info.getHeight()); //绘制文字
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
        auto child = currentMenu->getNextMenu();
        //如果当前页面指向的当前item没有后继 那就返回false
        if (child == nullptr) {
            popInfo({0, {{"unreferenced page!", getUIConfig().mainFont}}}, 800);
            return false;
        }
        if (!child->onOpen()) {
            return false;
        }
        if (!child->childMenu.empty()) {

        } else if (!child->childWidget.empty()) {
            for (auto &item: child->childWidget) {
                if (!item->onOpen()) {
                    break;
                }
            }
            return false;
        } else {
            popInfo({0, {{"empty page!", getUIConfig().mainFont}}}, 800);
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
            popInfo({0, {{"unreferenced page!", getUIConfig().mainFont}}}, 600);
            return false;
        }
        if (currentMenu->getPreview()->getItemNum() == 0) {
            popInfo({0, {{"empty page!", getUIConfig().mainFont}}}, 600);
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

    key::keyIndex Launcher::getKey() {
        static Clocker key_clocker;
        auto value = key::KEY_NUM;
        if (key_clocker.currentDuration() > 50) {
            if (HAL::keyScan()) {
                if (HAL::getKeyMap()[key::KEY_PREV] == key::CLICK) {
                    value = key::KEY_PREV;
                } else if (HAL::getKeyMap()[key::KEY_NEXT] == key::CLICK) {
                    value = key::KEY_NEXT;
                } else if (HAL::getKeyMap()[key::KEY_CONFIRM] == key::CLICK) {
                    value = key::KEY_CONFIRM;
                } else if (HAL::getKeyMap()[key::KEY_CANCEL] == key::CLICK) {
                    value = key::KEY_CANCEL;
                }
            }
            key_clocker.reset();
        }
        return value;
    }

    void Launcher::update(bool clear, const std::function<void(Clocker &clocker, key::keyIndex active_key)> &render) {
        static Clocker render_clocker;
        render_clocker.waitUntil(getUIConfig().perFrameMills);
        render_clocker.next();
        if (clear) HAL::canvasClear();
        render(render_clocker, getKey());
        HAL::canvasUpdate();
    }

    void Launcher::update() {
        update(true, [&](Clocker &render_clocker, key::keyIndex active_key) {
            currentMenu->render(camera->getPosition(), render_clocker);
            if (currentWidget != nullptr) currentWidget->render(camera->getPosition());
            selector->render(camera->getPosition(), render_clocker);
            camera->update(currentMenu, selector, render_clocker);
            switch (active_key) {
                case key::KEY_PREV:
                    selector->goPreview(); // selector去到上一个项目
                    break;
                case key::KEY_NEXT:
                    selector->goNext(); //selector去到下一个项目
                    break;
                case key::KEY_CONFIRM: {
                    render_clocker.pause();
                    open(); //打开当前项目
                    render_clocker.start();
                }
                    break;
                case key::KEY_CANCEL:
                    close(); //退出当前项目
                    break;
                case key::KEY_NUM:
                    break;
            }
        });
    }

    void Launcher::progress(float width, TextBox *text, const float *percentage,
                            std::function<bool(Clocker &, key::keyIndex)> render_callback) {
        float progressOffset = 0;
        auto sideBarAnimationSpeed = getUIConfig().progressInfAnimationSpeed;
        for (bool render_loop = true; render_loop;) {
            update(false, [&](Clocker &render_clocker, key::keyIndex active_key) {
                render_clocker.pause();
                if (!render_callback(render_clocker, active_key)) {
                    render_clocker.start();
                    render_loop = false;
                    return;
                }
                render_clocker.start();
                float textHeight = 0;
                if (text) {
                    textHeight = text->getHeight();
                }
                auto progressInnerPadding = getUIConfig().progressInnerPadding;
                auto progressHeight = getUIConfig().progressHeight;
                auto progressBoxHeight = textHeight + progressHeight + progressInnerPadding * 2 + 2 * 4;
                auto progressBoxY = (HAL::getSystemConfig().screenHeight - progressBoxHeight) / 2;
                auto progressBoxWidth = width + progressInnerPadding * 2 + 2 * 4;
                auto progressBoxX = (HAL::getSystemConfig().screenWeight - progressBoxWidth) / 2;
                //绘制消息框
                HAL::setDrawType(0);
                HAL::drawRBox(progressBoxX, progressBoxY, progressBoxWidth, progressBoxHeight, getUIConfig().popRadius);
                HAL::setDrawType(1);  //反色显示
                HAL::drawRFrame(progressBoxX + 1, progressBoxY + 1, progressBoxWidth - 2, progressBoxHeight - 2,
                                getUIConfig().popRadius);  //绘制一个圆角矩形
                //绘制进度条框
                HAL::setDrawType(0);
                HAL::drawRBox(progressBoxX + progressInnerPadding + 2, progressBoxY + progressInnerPadding + 2,
                              width + 4, progressHeight + 4, getUIConfig().popRadius);
                HAL::setDrawType(1);  //反色显示
                HAL::drawRFrame(progressBoxX + progressInnerPadding + 3, progressBoxY + progressInnerPadding + 3,
                                width + 2, progressHeight + 2, getUIConfig().popRadius);  //绘制一个圆角矩形

                if (percentage) {
                    //绘制固定进度条
                    auto filledWidth = width * (*percentage) / 100;
                    HAL::drawBox(progressBoxX + progressInnerPadding + 4, progressBoxY + progressInnerPadding + 4,
                                 filledWidth, progressHeight);
                } else {
                    //绘制无限滚动条
                    auto filledWidth = width / 4;
                    HAL::drawBox(progressOffset + progressBoxX + progressInnerPadding + 4,
                                 progressBoxY + progressInnerPadding + 4, filledWidth, progressHeight);
                    progressOffset += render_clocker.lastDuration() / 1000.0f * sideBarAnimationSpeed;
                    if (filledWidth + progressOffset >= width) {
                        //向右移动
                        progressOffset = width - filledWidth;
                        sideBarAnimationSpeed *= -1;
                    } else if (progressOffset <= 0) {
                        //向左移动
                        progressOffset = 0;
                        sideBarAnimationSpeed *= -1;
                    }
                }
                if (text) {
                    text->draw(progressBoxX + progressInnerPadding + 4,
                               progressBoxY + text->getHeight() + progressInnerPadding * 2 + 6);
                }
            });
        }
    }
    static std::string NUMBER_UNITS[]{" ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    std::string Launcher::numbersChoose(const unsigned char* font) {
        std::string result;
        int current_index = 1;
        HAL::setFont(font);
        auto number_height = HAL::getFontHeight();
        auto number_width = HAL::getFontWidth(NUMBER_UNITS[0]);
        auto number_input_margin_x = getUIConfig().number_input_margin_x;
        auto number_input_height =  float(number_height) + 4 * 2 + 2;
        auto screenWeight = float(HAL::getSystemConfig().screenWeight);
        auto number_input_width = screenWeight - number_input_margin_x * 2;
        auto screenHeight = float(HAL::getSystemConfig().screenHeight);
        auto number_input_y = (screenHeight - number_input_height) / 2;
        //绘制输入框
        HAL::setDrawType(0);
        HAL::drawRBox(number_input_margin_x, number_input_y, number_input_width, number_input_height, getUIConfig().popRadius);
        HAL::setDrawType(1);  //反色显示
        HAL::drawRFrame(number_input_margin_x + 1, number_input_y + 1, number_input_width - 2, number_input_height - 2,
                        getUIConfig().popRadius);  //绘制一个圆角矩形
        //绘制输入框内框
        int max_number_num = 0;
        float number_input_inner_width = 0;
        auto max_number_input_inner_width = number_input_width - 4 * 2 - 2;
        while (true) {
            float width =  HAL::getFontWidth(result);
            if(width < max_number_input_inner_width) {
                max_number_num = result.length();
                number_input_inner_width = width;
            } else {
                break;
            }
            result.append("0");
        }
        result.clear();
        auto number_input_inner_height = number_height + 4 + 1;
        auto number_input_inner_x = (screenWeight - number_input_inner_width) / 2 - 3;
        auto last_passed_time = 0;
        auto show_input_line = true;
        for (bool render_loop = true; render_loop;) {
            update(false, [&](Clocker &render_clocker, key::keyIndex active_key) {
                HAL::setDrawType(0);
                HAL::drawRBox(number_input_inner_x, number_input_y + 2, number_input_inner_width + 6, number_input_inner_height, getUIConfig().popRadius);
                HAL::setDrawType(1);  //反色显示
                HAL::drawRFrame(number_input_inner_x + 1, number_input_y + 3, number_input_inner_width + 4, number_input_inner_height - 2,
                                getUIConfig().popRadius);  //绘制一个圆角矩形
                HAL::setFont(font);
                // 绘制已输入内容
                auto text = result;
                if(result.length() < max_number_num){ // 超出输入框宽度，不绘制待输入内容
                    text.append(NUMBER_UNITS[current_index]);
                    // 绘制正在输入内容
                    auto width = float(HAL::getFontWidth(text));
                    last_passed_time += render_clocker.lastDuration();
                    if(last_passed_time > 500) {
                        last_passed_time = 0;
                        show_input_line = !show_input_line;
                    }
                    if(show_input_line) {
                        HAL::drawHLine(number_input_inner_x + width + 2 - number_width, number_input_y + 4 + number_height, number_width+1);
                    }
                }
                HAL::drawChinese(number_input_inner_x + 3, number_input_y + 2 + number_height, text);
                if(active_key == key::KEY_CONFIRM) {
                    if(result.length() < max_number_num) {
                        if(current_index == 0) {
                            render_loop = false;
                        } else {
                            result.append(NUMBER_UNITS[current_index]);
                            current_index = 1;
                        }
                    } else {
                        render_loop = false;
                    }
                } else if(active_key == key::KEY_PREV) {
                    current_index = (current_index + 10) % 11;
                } else if(active_key == key::KEY_NEXT) {
                    current_index = (current_index + 1) % 11;
                } else if(active_key == key::KEY_CANCEL) {
                    if(result.empty()) {
                        render_loop = false;
                    } else {
                        auto last = result[result.length() -1];
                        if(last == ' ') {
                            current_index = 0;
                        } else {
                            current_index = last - '0' + 1;
                        }
                        result.pop_back();
                    }
                }
            });
        }
        return result;
    }
}