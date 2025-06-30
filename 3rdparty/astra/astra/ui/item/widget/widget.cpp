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

    static bool buildStringPrefixWidthAndPos(const std::string &text, std::vector<int> &_width, std::vector<int> &_pos)
    {
        auto getWidth = [](unsigned char byte)-> int
        {
            if ((byte & 0xF8) == 0xF0) return 4; // 4字节字符
            if ((byte & 0xF0) == 0xE0) return 3; // 3字节字符
            if ((byte & 0xE0) == 0xC0) return 2; // 2字节字符
            return 1;
        };
        _width.push_back(0);
        _pos.push_back(0);
        for (int i = 0; i < text.size();)
        {
            auto width = getWidth(text[i]);
            if (i + width > text.size())
            {
                return false;
            }
            i += width;
            _width.push_back(HAL::getFontWidth(text.substr(0, i)));
            _pos.push_back(i);
        }
        return true;
    }

    Text::Text(std::string _text, const unsigned char *_font) : m_text(std::move(_text)), m_font(_font) {
        HAL::setFont(m_font);
        buildStringPrefixWidthAndPos(m_text, _width, _pos);
        m_w = _width.empty() ? 0 : _width[_width.size() -1];
        m_h = HAL::getFontHeight();
        max_w = m_w;
        resetAnimate();
    }

    float Text::getHeight() const {
        return m_h;
    }

    float Text::getWidth() const {
        if (m_w > max_w)
        {
            return max_w;
        }
        return m_w;
    }

    void Text::setMaxWidth(float _max_w)
    {
        if (_max_w != max_w)
        {
            max_w = _max_w;
            resetAnimate();
        }
    }

    void Text::draw(float _x, float _y, Clocker &clocker) {
        HAL::setFont(m_font);
        HAL::drawChinese(_x, _y, computeAnimate(clocker));
    }

    void Text::resetAnimate()
    {
        animate_offset = 0;
        animate_offset_max = 0;
        animate_duration_count = 0;
        animate_forward = true;
        int offset = m_w - max_w;
        if (offset > 0)
        {
            for (int i = 0; i < _width.size(); i++)
            {
                if (_width[i] > offset)
                {
                    animate_offset_max = i;
                    return;
                }
            }
        }
    }

    std::string Text::computeAnimate(Clocker& clocker)
    {
        if (animate_offset_max > 0)
        {
            animate_duration_count += clocker.lastDuration();
            if (animate_duration_count > getUIConfig().text_flow_speed)
            {
                animate_duration_count = 0;
                if (animate_offset + 1 >= animate_offset_max)
                {
                    animate_offset = 0;
                    animate_forward = !animate_forward;
                } else
                {
                    animate_offset += 1;
                }
            }
            int prefix_pos = animate_offset;
            if (!animate_forward)
            {
                prefix_pos = animate_offset_max - prefix_pos;
            }
            int end_pos = -1;
            for (int i=prefix_pos + 1; i < _pos.size(); i++)
            {
                if (_width[i] - _width[prefix_pos] > max_w)
                {
                    end_pos = i - 1;
                    break;
                }
            }
            if (end_pos == -1)
            {
                return m_text.substr(_pos[prefix_pos]);
            }
            return m_text.substr(_pos[prefix_pos], _pos[end_pos] - _pos[prefix_pos]);
        }
        return m_text;
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

