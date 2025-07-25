#include "astra/ui/item/widget/widget.h"

#include <cmath>
#include <utility>
#include "astra/ui/launcher.h"

namespace astra {

    CheckBox::CheckBox(bool default_value, std::function<void(bool&)> on_change) : m_on_change(std::move(on_change)) {
        value = default_value;
        if (value) isCheck = true;
        else isCheck = false;
        this->parent = nullptr;
    }

    void CheckBox::init() {
        if (value) isCheck = true;
        else isCheck = false;
    }

    void CheckBox::deInit() {
        delete this;
    }

    void CheckBox::renderIndicator(float _x, float _y, const std::vector<float> &_camera) {
        Item::updateConfig();
        //绘制外框
        HAL::setDrawType(1);
        HAL::drawRFrame(_x + _camera[0],
                        _y + _camera[1],
                        astraConfig.checkBoxWidth,
                        astraConfig.checkBoxHeight,
                        astraConfig.checkBoxRadius);
        if (isCheck) //绘制复选框内的点
            HAL::drawBox(_x + _camera[0] + astraConfig.checkBoxWidth / 4,
                         _y + _camera[1] + astraConfig.checkBoxHeight / 4,
                         astraConfig.checkBoxWidth / 2,
                         astraConfig.checkBoxHeight / 2);
    }

    bool CheckBox::onOpen() {
        value = !value;
        isCheck = !isCheck;
        if (m_on_change) {
            m_on_change(isCheck);
        }
        return true;
    }

    Slider::Slider(const std::string &_title,
                   unsigned char _min,
                   unsigned char _max,
                   unsigned char _step,
                   unsigned char &_value) {
        title = _title;
        maxLength = 0;
        min = _min;
        max = _max;
        step = _step;
        value = _value;
        lengthIndicator = 0;
        this->parent = nullptr;

        if (value > max) valueOverflow = true;
        else valueOverflow = false;
    }

    unsigned char Slider::add() {
        value += step;
        return this->value;
    }

    unsigned char Slider::sub() {
        value -= step;
        return this->value;
    }

    void Slider::init() {
        maxLength = std::floor(HAL::getSystemConfig().screenWeight * 0.6);
        position.lTrg = std::floor(((float) (value - min) / (max - min)) * maxLength); //计算目标长度
        lengthIndicator = std::round(((float) (value - min) / (max - min)) * 6);  //映射在0-6个像素之间
        if (valueOverflow) {
            position.lTrg = maxLength;
            lengthIndicator = 6;
        }
    }

    void Slider::deInit() {
        delete this;
    }

    void Slider::renderIndicator(float _x, float _y, const std::vector<float> &_camera) {
        Item::updateConfig();
        HAL::setDrawType(1);
        HAL::drawRFrame(_x + _camera[0] - 1, _y + _camera[1] - 1, 10, 8, 1);
        HAL::drawBox(_x + _camera[0] + 1, _y + _camera[1] + 1, lengthIndicator, 4);
    }

    void Slider::render(const std::vector<float> &_camera) {
        Widget::render(_camera);
    }

    Text::Text(std::string _text, const unsigned char *_font) : m_text_(std::move(_text)), m_font_(_font) {
        HAL::setFont(m_font_);
        m_w_ = HAL::getFontWidth(m_text_);;
        m_h_ = HAL::getFontHeight();
        resetAnimate();
    }

    float Text::getHeight() const {
        return m_h_;
    }

    float Text::getWidth() const {
        if (animate_offset_max_ > 0)
        {
            return m_max_w_;
        }
        return m_w_;
    }

    void Text::setMaxWidth(float max_w)
    {
        if (max_w != m_max_w_)
        {
            m_max_w_ = max_w;
            animate_offset_max_ = m_w_ - m_max_w_;
            resetAnimate();
        }
    }

    void Text::draw(float _x, float _y, Clocker &clocker) {
        HAL::setFont(m_font_);
        if (animate_offset_max_ > 0)
        {
            if (animate_boundary_wait_time_ > 0)
            {
                animate_boundary_wait_time_-=clocker.lastDuration();
            }
            if (animate_boundary_wait_time_ <= 0)
            {
                Animation::moveUniform(animate_offset_, animate_offset_max_, getUIConfig().text_flow_speed, clocker);
            }
            HAL::setClipWindow(_x, _y - m_h_ + 2, _x + m_max_w_, _y + 2);
            if (animate_offset_ >= animate_offset_max_)
            {
                animate_offset_ = 0;
                animate_boundary_wait_time_ = getUIConfig().text_flow_boundary_wait_time;
                animate_forward_ = !animate_forward_;
            }
            if (animate_forward_)
            {
                _x -= animate_offset_max_ - animate_offset_;
            } else
            {
                _x -= animate_offset_;
            }
        }
        HAL::drawChinese(_x, _y, m_text_);
        if (animate_offset_max_ > 0)
        {
            HAL::setMaxClipWindow();
        }
    }

    void Text::resetAnimate()
    {
        animate_offset_ = 0;
        animate_forward_ = true;
    }

    TextBox::TextBox(float margin, const std::initializer_list<Text> &_texts): m_margin(margin), m_texts(_texts), m_h(0), m_w(0) {
        for (auto &text: m_texts) {
            m_h += text.getHeight() + m_margin;
            m_w = fmax(m_w, text.getWidth());
        }
        m_h -= margin;
    }

    void TextBox::draw(float _x, float _y, Clocker &clocker) {
        float y = _y;
        auto it = m_texts.end();
        while (it != m_texts.begin()) {
            --it;
            it->draw(_x, y, clocker);
            y -= it->getHeight() + m_margin;
        }
    }

    float TextBox::getWidth() const {
        return m_w;
    }

    float TextBox::getHeight() const {
        return m_h;
    }

    void TextBox::add(Text _text) {
        if(!m_texts.empty()) {
            m_h += m_margin;
        }
        _text.setMaxWidth(max_w);
        m_texts.emplace_back(std::move(_text));
        m_h += _text.getHeight();
        m_w = m_w < _text.getWidth() ? _text.getWidth() : m_w;
    }

    void TextBox::setText(int _index, const Text &_text) {
        if(_index < 0 || _index >= m_texts.size()) {
            return;
        }
        auto old = m_texts[_index];
        old.setMaxWidth(max_w);
        m_texts[_index] = _text;
        m_h  = m_h - old.getHeight() + _text.getHeight();
        updateWidth();
    }

    void TextBox::setMaxWidth(float _max_w)
    {
        if (max_w == _max_w)
        {
            return;
        }
        max_w = _max_w;
        m_w = 0;
        for (auto &item: m_texts)
        {
            item.setMaxWidth(_max_w);
            m_w = std::fmax(item.getWidth(), m_w);
        }
    }

    void TextBox::updateWidth() {
        m_w = 0;
        for (const auto &item: m_texts)
        {
            m_w = std::fmax(item.getWidth(), m_w);
        }
    }

    void TextBox::remove(int _index) {
        if(_index < 0 || _index >= m_texts.size()) {
            return;
        }
        auto item = m_texts.erase(m_texts.begin() + _index);
        if(!m_texts.empty()) {
            m_h -= item->getHeight() + m_margin;
        }
        updateWidth();
    }

    void TextBox::clear() {
        m_texts.clear();
        m_w = 0;
        m_h = 0;
    }
}

