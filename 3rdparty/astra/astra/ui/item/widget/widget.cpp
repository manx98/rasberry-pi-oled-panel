#include "astra/ui/item/widget/widget.h"

#include <cmath>
#include <utility>
#include "astra/ui/launcher.h"

namespace astra {

    CheckBox::CheckBox(bool default_value, std::function<void(bool)> on_change) : m_on_change(std::move(on_change)) {
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

    void CheckBox::render(const std::vector<float> &_camera) {
        //todo 选中复选框后弹出消息提醒 这玩意现在我倒觉得没啥必要 可以暂时不做
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

    Text::Text(std::string _text, const unsigned char *_font) : m_text(std::move(_text)), m_font(_font) {
        HAL::setFont(m_font);
        m_w = HAL::getFontWidth(m_text);
        m_h = HAL::getFontHeight();
    }

    float Text::getHeight() const {
        return m_h;
    }

    float Text::getWidth() const {
        return m_w;
    }

    void Text::draw(float _x, float _y) const {
        HAL::setFont(m_font);
        HAL::drawChinese(_x, _y, m_text);
    }

    TextBox::TextBox(float margin, const std::initializer_list<Text> &_texts): m_margin(margin), m_texts(_texts), m_h(0), m_w(0) {
        for (auto &text: m_texts) {
            m_h += text.getHeight() + m_margin;
            m_w = m_w < text.getWidth() ? text.getWidth() : m_w;
        }
        m_h -= margin;
    }
    void TextBox::draw(float _x, float _y) const {
        float y = _y;
        auto it = m_texts.end();
        while (it != m_texts.begin()) {
            --it;
            it->draw(_x, y);
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
        m_texts.emplace_back(std::move(_text));
        m_h += _text.getHeight();
        m_w = m_w < _text.getWidth() ? _text.getWidth() : m_w;
    }

    void TextBox::setText(int _index, const Text &_text) {
        if(_index < 0 || _index >= m_texts.size()) {
            return;
        }
        auto old = m_texts[_index];
        m_texts[_index] = _text;
        m_h  = m_h - old.getHeight() + _text.getHeight();
        updateWidth();
    }

    void TextBox::updateWidth() {
        m_w = 0;
        for (const auto &item: m_texts)
        {
            m_w = static_cast<float>(std::fmax(item.getWidth(), m_w));
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
}

