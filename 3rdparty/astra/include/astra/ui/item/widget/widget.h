#pragma once
#ifndef ASTRA_WIDGET__H
#define ASTRA_WIDGET__H

#include <functional>
#include <list>
#include "astra/ui/item/item.h"

namespace astra {
    enum WidgetType {
        WIDGET_TYPE_BASE,
        WIDGET_TYPE_CHECK_BOCK,
        WIDGET_TYPE_POP_UP,
        WIDGET_TYPE_SLIDER,
        WIDGET_TYPE_LIST,
        WIDGET_TYPE_TILE,
    };

    class Text {
    public:
        Text(std::string _text, const unsigned char *_font);
        [[nodiscard]] float getHeight() const;
        [[nodiscard]] float getWidth() const;
        void setMaxWidth(float _max_w);
        void draw(float _x, float _y, Clocker &clocker);
    private:
        void resetAnimate();
        std::string m_text_;
        const unsigned char *m_font_;
        float m_w_;
        float m_h_;
        float m_max_w_{0};
        float animate_offset_{0};
        float animate_boundary_wait_time_{0};
        float animate_offset_max_{0};
        bool animate_forward_{false};
    };

    class TextBox {
    private:
        std::vector<Text> m_texts;
        float m_w, m_h;
        float max_w{0};
        float m_margin{0};
        void updateWidth();
    public:
        TextBox(float margin, const std::initializer_list<Text> &_texts);
        void setText(int _index, const Text &_text);
        void setMaxWidth(float _max_w);
        void add(Text _text);
        void remove(int _index);
        void draw(float _x, float _y, Clocker &clocker);
        void clear();
        [[nodiscard]] float getWidth() const;
        [[nodiscard]] float getHeight() const;
    };

    class Widget : public Item {
    public:
        void *parent{};
        unsigned char value{};

    public:
        [[nodiscard]] virtual WidgetType getType() const { return WIDGET_TYPE_BASE; }

    public:
        Widget() = default;

    public:
        virtual void init() {}

        virtual void deInit() {}
        //open and close是启动器的事情 与控件无关

    public:
        //绘制控件在列表中的指示器
        virtual void renderIndicator(float _x, float _y, const std::vector<float> &_camera) {}

    public:
        virtual void render(const std::vector<float> &_camera) {}

        virtual bool onOpen() {
            return true;
        };
    };

    class CheckBox : public Widget {
    private:
        std::function<void(bool&)> m_on_change = nullptr;
    public:
        [[nodiscard]] WidgetType getType() const override { return WIDGET_TYPE_CHECK_BOCK; }

    private:
        bool isCheck;

    public:
        explicit CheckBox(bool default_value, std::function<void(bool&)> on_change);  //check box.
    public:
        void init() override;

        void deInit() override;

    public:
        void renderIndicator(float _x, float _y, const std::vector<float> &_camera) override;

        bool onOpen() override;
    };

    class Slider : public Widget {
    public:
        [[nodiscard]] WidgetType getType() const override { return WIDGET_TYPE_SLIDER; }

    public:
        typedef struct Position {
            float x, xTrg;
            float y, yTrg;

            float l, lTrg; //slider
        } Position;

        Position position{};

    private:
        std::string title;
        unsigned char maxLength;
        unsigned char min;
        unsigned char max;
        unsigned char step;

        bool valueOverflow;

        unsigned char lengthIndicator;

    public:
        Slider(const std::string &_title,
               unsigned char _min,
               unsigned char _max,
               unsigned char _step,
               unsigned char &_value);  //slider.

    public:
        unsigned char add();

        unsigned char sub();

    public:
        void init() override;

        void deInit() override;

    public:
        void renderIndicator(float _x, float _y, const std::vector<float> &_camera) override;

    public:
        void render(const std::vector<float> &_camera) override;
    };
}

#endif //ASTRA_WIDGET__H