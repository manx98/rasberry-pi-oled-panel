//
// Created by Fir on 2024/1/21.
//

#pragma once
#ifndef ASTRA_ASTRA__H
#define ASTRA_ASTRA__H

#include "cstdint"
#include "string"
#include <utility>
#include <vector>
#include "astra/ui/item/item.h"
#include "astra/ui/item/widget/widget.h"
#include "astra/utils/clocker.h"

namespace astra {

    class Menu : public Item {
    public:
        virtual ~Menu() = default;

        explicit Menu(TextBox _title) : title(std::move(_title)) {};

        [[nodiscard]] virtual WidgetType getType() const { return WIDGET_TYPE_BASE; }

        class Event {
        public:
            virtual ~Event() = default;

            virtual bool beforeOpen(Menu *current) = 0;

            virtual bool beforeRender(Menu *current, const std::vector<float> &_camera, Clocker &clocker) = 0;
        };

    public:
        std::vector<float> cameraPosMemory = {};

        void rememberCameraPos(const std::vector<float> &_camera) {
            cameraPosMemory = _camera;
            cameraPosMemoryFlag = true;
        }

        [[nodiscard]] std::vector<float> getCameraMemoryPos() const { return cameraPosMemory; }

        void resetCameraMemoryPos() { cameraPosMemory = {0, 0}; }

        //编写一个变量 指示该页面到底有没有记忆
        bool cameraPosMemoryFlag = false;

        virtual float getWidth() = 0;

        virtual float getHeight() = 0;
    public:
        //存储其在父页面中的位置
        //list中就是每一项对应的坐标 tile中就是每一个图片的坐标
        typedef struct Position {
            float x, xTrg;
            float y, yTrg;
        } Position;

        Position position{};

        [[nodiscard]] Position getItemPosition(unsigned char _index) const;

        virtual void childPosInit(const std::vector<float> &_camera) {}

        virtual void forePosInit() {}

        virtual bool onOpen() {
            return true;
        };
    public:
        TextBox title;
        std::vector<unsigned char> pic;

    protected:
        std::vector<unsigned char> picDefault = {};

        [[nodiscard]] std::vector<unsigned char> generateDefaultPic();

    public:
        Menu *parent{};
        std::vector<Menu *> childMenu; //allow widget and menu.
        std::vector<Widget *> childWidget;
        unsigned char selectIndex{};

        [[nodiscard]] unsigned char getItemNum() const;

        [[nodiscard]] Menu *getNextMenu() const;  //启动器调用该方法来获取下一个页面
        [[nodiscard]] Menu *getPreview() const;

    public:
        bool initFlag = false;

    public:
        virtual void init(const std::vector<float> &_camera); //每次打开页面都要调用一次
        void deInit(); //每次关闭页面都要调用一次

    public:
        void doRender(const std::vector<float> &_camera, Clocker &clocker);
        virtual void render(const std::vector<float> &_camera, Clocker &clocker) {}  //render all child item.
    private:
        std::vector<Menu *> delayRemoveItem;
    public:
        bool addItem(Menu *_page);

        bool addItem(Menu *_page, Widget *_anyWidget); //新建一个带有控件的列表项

        void removeItem(Menu *_page);

        void clear();

        void cleanDelayRemoveItem();
    };

    class List : public Menu {
    public:
        [[nodiscard]] WidgetType getType() const override { return WIDGET_TYPE_LIST; }

    public:
        //前景元素的坐标
        typedef struct PositionForeground {
            float hBar, hBarTrg;  //进度条高度
            float xBar, xBarTrg;  //进度条x坐标
        } PositionForeground;

        PositionForeground positionForeground{};

        float getWidth() override;

        float getHeight() override;
    public:
        void childPosInit(const std::vector<float> &_camera) override;

        void forePosInit() override;

        explicit List(const TextBox &_title);

        List(const TextBox &_title, Event *event);

        List(const TextBox &_title, const std::vector<unsigned char> &_pic);

        List(const TextBox &_title, const std::vector<unsigned char> &_pic, Event *event);

        ~List() override;

    public:
        std::vector<float> boundary = {0, float(systemConfig.screenHeight)};

        [[nodiscard]] std::vector<float> getBoundary() const { return boundary; }

        void refreshBoundary(float _l, float _r) { boundary = {_l, _r}; }

        bool onOpen() override;

        Menu* getSelected();

    public:
        void render(const std::vector<float> &_camera, Clocker &clocker) override;

        void init(const std::vector<float>& _camera) override;

    private:
        void updateTitleMaxWidth();
        Event *m_event = nullptr;
    };

    class Tile : public Menu {
    public:
        [[nodiscard]] WidgetType getType() const override { return WIDGET_TYPE_TILE; }

    public:
        //前景元素的坐标
        typedef struct PositionForeground {
            float wBar, wBarTrg;  //进度条宽度
            float yBar, yBarTrg;  //进度条y坐标
        } PositionForeground;

        PositionForeground positionForeground{};

    public:
        void childPosInit(const std::vector<float> &_camera) override;

        void forePosInit() override;

        explicit Tile(const TextBox &_title);

        Tile(const TextBox &_title, const std::vector<unsigned char> &_pic);

        ~Tile() override = default;

    public:
        void render(const std::vector<float> &_camera, Clocker &clocker) override;

        float getWidth() override;

        float getHeight() override;
    };

}

#endif //ASTRA_ASTRA__H