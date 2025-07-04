//
// Created by Fir on 2024/1/21.
//

//每个列表应该有 1. 文本（需要显示的内容） 2. 跳转向量表（对应1，每一行都要跳转到哪里）
//其中文本以string形式 向量表以vector形式 以类的形式整合在一起 编写相应的构造函数 调用构造函数即是新建一个页面
//通过最后一项跳转向量表可以判断其类型 比如跳转至下一页 或者返回上一级 或者是复选框 或者是弹窗 或者是侧边栏弹窗 等等
//做一个astra类作为总的框架 在ui命名空间里

//分为
//驱动层 - 界面层（包括各种列表 磁铁 复选框 侧边栏 弹窗等） - 处理层（各种坐标的变换等）

//传进去的有三个东西 第一个是文字（要显示的东西） 第二个是类别（普通列表 跳转列表 复选框） 第三个是特征向量（指向复选框和弹窗对应的参数 要跳转的地方等）

#include "astra/ui/item/menu/menu.h"
#include "astra/ui/launcher.h"

/**
 *     ·  ·     ·   ·
 *  ·   ·  ·  ·   ·
 *  循此苦旅，直抵群星。
 *  ad astra per aspera.
 * happy Chinese new year!
 *      新年快乐！
 * ·   ·   ·      ·
 *   ·   ·    · ·   ·
 */

namespace astra {
    Menu::Position Menu::getItemPosition(unsigned char _index) const { return childMenu[_index]->position; }

    std::vector<unsigned char> Menu::generateDefaultPic() {
        this->picDefault.resize(120, 0xFF);
        return this->picDefault;
    }

    unsigned char Menu::getItemNum() const { return childMenu.size(); }

    Menu *Menu::getNextMenu() const { return childMenu[selectIndex]; }

    Menu *Menu::getPreview() const { return parent; }

    void Menu::init(const std::vector<float> &_camera) {}

    void Menu::deInit() {
        Animation::exit();
    }

    bool Menu::addItem(Menu *_page) {
        if (_page == nullptr) return false;
        if (!_page->childWidget.empty()) return false;
        _page->parent = this;
        this->childMenu.push_back(_page);
        this->forePosInit();
        return true;
    }

    bool Menu::addItem(Menu *_page, Widget *_anyWidget) {
        if (_anyWidget == nullptr) return false;
        if (this->addItem(_page)) {
            _page->childWidget.push_back(_anyWidget);
            _anyWidget->parent = _page;
            _anyWidget->init();
            return true;
        } else return false;
    }

    void Menu::clear() {
        for (const auto &item: childMenu) {
            delete item;
        }
        childMenu.clear();
        for (const auto &item: childWidget) {
            delete item;
        }
        childWidget.clear();
    }

    void Menu::removeItem(Menu *_page) {
        for (auto it = childMenu.begin(); it != childMenu.end(); ++it) {
            auto item = *it;
            if (item == _page) {
                childMenu.erase(it);
                delayRemoveItem.push_back(item);
                if(childMenu.size() >= selectIndex) {
                    Launcher::getSelector()->goPreview();
                } else {
                    Launcher::getSelector()->setPosition();
                }
                break;
            }
        }
    }

    void Menu::cleanDelayRemoveItem() {
        if(!delayRemoveItem.empty()) {
            for (auto &item: delayRemoveItem) {
                delete item;
            }
            delayRemoveItem.clear();
        }
    }

    void Menu::doRender(const std::vector<float> &_camera, Clocker &clocker) {
        cleanDelayRemoveItem();
        render(_camera, clocker);
    }

    void List::childPosInit(const std::vector<float> &_camera) {
        float top = 0;
        for (auto _iter: childMenu) {
            _iter->position.x = astraConfig.listTextMargin;
            _iter->position.xTrg = astraConfig.listTextMargin;
            _iter->position.yTrg = top;

            top += _iter->title.getHeight() + astraConfig.listTextMargin;

            //受展开开关影响的坐标初始化
            //根页面有开场动画 所以不需要从头展开
            if (_iter->parent->parent == nullptr) {
                _iter->position.y = _iter->position.yTrg;
                continue;
            }
            if (astraConfig.listUnfold) {
                _iter->position.y = _camera[1] - _iter->title.getHeight();
                continue;
            } //m_text unfold from top.
        }
    }

    void List::forePosInit() {
        positionForeground.xBarTrg = systemConfig.screenWeight - astraConfig.listBarWeight;

        //受展开开关影响的坐标初始化
        if (astraConfig.listUnfold) positionForeground.hBar = 0;  //bar unfold from top.
        else positionForeground.hBar = positionForeground.hBarTrg;

        //始终执行的坐标初始化
        positionForeground.xBar = systemConfig.screenWeight;
    }

    List::List(const TextBox &_title) : Menu(_title) {
        this->pic = generateDefaultPic();

        this->selectIndex = 0;

        this->parent = nullptr;
        this->childMenu.clear();
        this->childWidget.clear();

        this->position = {};
        this->positionForeground = {};
        updateTitleMaxWidth();
    }

    List::List(const TextBox &_title, const std::vector<unsigned char> &_pic) : Menu(_title) {
        this->pic = _pic;

        this->selectIndex = 0;

        this->parent = nullptr;
        this->childMenu.clear();
        this->childWidget.clear();

        this->position = {};
        this->positionForeground = {};
        updateTitleMaxWidth();
    }

    List::List(const TextBox &_title, const std::vector<unsigned char> &_pic, Event *event) : List(_title, _pic) {
        m_event = event;
        updateTitleMaxWidth();
    }

    void List::render(const std::vector<float> &_camera, Clocker &clocker) {
        if (m_event && !m_event->beforeRender(this, _camera, clocker)) {
            return;
        }
        Item::updateConfig();

        HAL::setDrawType(1);
        //allow x > screen height, y > screen weight.
        //scan all children, draw m_text and widget on the list.
        float total_height = 0;
        float select_height = 0;
        int index = 0;
        for (auto _iter: childMenu) {
            //绘制控件在列表中的指示器
            if (!_iter->childWidget.empty()) {
                for (auto _widget: _iter->childWidget) {
                    _widget->renderIndicator(
                            systemConfig.screenWeight - astraConfig.checkBoxRightMargin - astraConfig.checkBoxWidth,
                            _iter->position.y + astraConfig.checkBoxTopMargin,
                            _camera);
                }
            }
            total_height += _iter->title.getHeight();
            if (index <= selectIndex) {
                select_height += _iter->title.getHeight();
            }
            //绘制文字
            _iter->title.draw(_iter->position.x + _camera[0],
                              _iter->position.y + _iter->title.getHeight() + _camera[1], clocker);
            //这里的yTrg在addItem的时候就已经确定了
            Animation::move(_iter->position.y, _iter->position.yTrg, astraConfig.listAnimationSpeed, clocker);
            index++;
        }

        //draw bar.
        positionForeground.hBarTrg = systemConfig.screenHeight * (select_height / total_height);
        //画指示线
        HAL::drawHLine(systemConfig.screenWeight - astraConfig.listBarWeight, 0, astraConfig.listBarWeight);
        HAL::drawHLine(systemConfig.screenWeight - astraConfig.listBarWeight,
                       systemConfig.screenHeight - 1,
                       astraConfig.listBarWeight);
        HAL::drawVLine(systemConfig.screenWeight - ceil((float) astraConfig.listBarWeight / 2.0f),
                       0,
                       systemConfig.screenHeight);
        //draw bar.
        HAL::drawBox(positionForeground.xBar, 0, astraConfig.listBarWeight, positionForeground.hBar);

        //light mode.
        if (astraConfig.lightMode) {
            HAL::setDrawType(2);
            HAL::drawBox(0, 0, systemConfig.screenWeight, systemConfig.screenHeight);
            HAL::setDrawType(1);
        }

        Animation::move(positionForeground.hBar, positionForeground.hBarTrg, astraConfig.listAnimationSpeed, clocker);
        Animation::move(positionForeground.xBar, positionForeground.xBarTrg, astraConfig.listAnimationSpeed, clocker);
    }

    void List::init(const std::vector<float>& _camera)
    {
        updateTitleMaxWidth();
    }

    void List::updateTitleMaxWidth()
    {
        if (childWidget.empty())
        {
            title.setMaxWidth(systemConfig.screenWeight
                               - astraConfig.listBarWeight
                               - astraConfig.checkBoxRightMargin
                               - astraConfig.listTextMargin * 2);
        } else
        {
            title.setMaxWidth(systemConfig.screenWeight
                            - astraConfig.listBarWeight
                            - astraConfig.checkBoxRightMargin
                            - astraConfig.checkBoxWidth
                            - astraConfig.listTextMargin * 2);
        }
    }

    bool List::onOpen() {
        if (m_event) {
            return m_event->beforeOpen(this);
        } else {
            return true;
        }
    }

    Menu* List::getSelected(){
        if(childMenu.empty()) {
            return nullptr;
        }
        if(selectIndex >= childMenu.size()) {
            selectIndex = 0;
        }
        return childMenu[selectIndex];
    }

    float List::getWidth() {
        return title.getWidth() + astraConfig.listTextMargin * 2;
    }

    float List::getHeight() {
        return title.getHeight() + astraConfig.listTextMargin;
    }

    List::List(const TextBox &_title, Menu::Event *event) : Menu(_title), m_event(event) {
        this->pic = generateDefaultPic();
        this->selectIndex = 0;

        this->parent = nullptr;
        this->childMenu.clear();
        this->childWidget.clear();

        this->position = {};
        this->positionForeground = {};
    }

    List::~List() {
        delete m_event;
    }

    void Tile::childPosInit(const std::vector<float> &_camera) {
        unsigned char _index = 0;

        for (auto _iter: childMenu) {
            _iter->position.y = 0;
            _iter->position.xTrg = systemConfig.screenWeight / 2 - astraConfig.tilePicWidth / 2 +
                                   (_index) * (astraConfig.tilePicMargin + astraConfig.tilePicWidth);
            _iter->position.yTrg = astraConfig.tilePicTopMargin;

            _index++;

            if (_iter->parent->parent == nullptr) {
                _iter->position.x = _iter->position.xTrg;
                continue;
            }
            if (astraConfig.tileUnfold) {
                _iter->position.x = _camera[0] - astraConfig.tilePicWidth;
                continue;
            } //unfold from left.
        }
    }

    void Tile::forePosInit() {
        positionForeground.yBarTrg = 0;
        if (astraConfig.tileUnfold) positionForeground.wBar = 0;  //bar unfold from left.
        else positionForeground.wBar = positionForeground.wBarTrg;

        //顶部进度条的从上方滑入的初始化
        positionForeground.yBar = 0 - astraConfig.tileBarHeight; //注意这里是坐标从屏幕外滑入 而不是height从0变大
    }

    Tile::Tile(const TextBox &_title) : Menu(_title) {
        this->pic = generateDefaultPic();

        this->selectIndex = 0;

        this->parent = nullptr;
        this->childMenu.clear();
        this->childWidget.clear();

        this->position = {};
        this->positionForeground = {};
    }

    Tile::Tile(const TextBox &_title, const std::vector<unsigned char> &_pic) : Menu(_title) {
        this->pic = _pic;

        this->selectIndex = 0;

        this->parent = nullptr;
        this->childMenu.clear();
        this->childWidget.clear();

        this->position = {};
        this->positionForeground = {};
    }

    void Tile::render(const std::vector<float> &_camera, Clocker &clocker) {
        Item::updateConfig();

        HAL::setDrawType(1);
        //draw pic.
        for (auto _iter: childMenu) {
            HAL::drawBMP(_iter->position.x + _camera[0],
                         astraConfig.tilePicTopMargin + _camera[1],
                         astraConfig.tilePicWidth,
                         astraConfig.tilePicHeight,
                         _iter->pic.data());
            //这里的xTrg在addItem的时候就已经确定了
            Animation::move(_iter->position.x,
                            _iter->position.xTrg,
                            astraConfig.tileAnimationSpeed, clocker);
        }

        //draw bar.
        //在屏幕最上方 两个像素高
        positionForeground.wBarTrg = (selectIndex + 1) * ((float) systemConfig.screenWeight / getItemNum());
        HAL::drawBox(0, positionForeground.yBar, positionForeground.wBar, astraConfig.tileBarHeight);

        Animation::move(positionForeground.wBar, positionForeground.wBarTrg, astraConfig.tileAnimationSpeed, clocker);
        Animation::move(positionForeground.yBar, positionForeground.yBarTrg, astraConfig.tileAnimationSpeed, clocker);
    }

    float Tile::getWidth() {
        return 0;
    }

    float Tile::getHeight() {
        return 0;
    }
}
