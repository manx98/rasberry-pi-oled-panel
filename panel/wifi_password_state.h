#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

// Two-button Wi-Fi password keyboard state, independent of the OLED HAL.
// Short Previous/Next select; long Confirm acts; long Cancel returns/undoes.
class WifiPasswordState {
public:
    enum class Key { Previous, Next, Confirm, Cancel };
    enum class Stage { Categories, Characters, EditPassword, Submitted, Cancelled };
    enum class Category {
        Lower, Upper, Digits, Symbols, ShowPassword, Delete, Connect, Cancel, Count
    };

    explicit WifiPasswordState(std::size_t minimumLength = 8, std::size_t maximumLength = 63,
                               bool legacyWep = false)
        : minimumLength_(minimumLength), maximumLength_(maximumLength), legacyWep_(legacyWep) {}
    ~WifiPasswordState() { clearPassword(); }

    Stage stage() const { return stage_; }
    Category category() const { return static_cast<Category>(category_); }
    std::size_t categoryPosition() const { return category_; }
    std::size_t characterPosition() const { return characterPositions_[category_]; }
    const std::string &password() const { return password_; }
    const std::string &error() const { return error_; }
    bool showPassword() const { return showPassword_; }
    std::string previewPassword() const { return showPassword_ ? password_ : masked(); }
    std::string masked() const { return std::string(password_.size(), '*'); }
    bool done() const { return stage_ == Stage::Submitted || stage_ == Stage::Cancelled; }
    bool submitted() const { return stage_ == Stage::Submitted; }

    static std::string characters(Category kind) {
        switch (kind) {
            case Category::Lower: return "abcdefghijklmnopqrstuvwxyz";
            case Category::Upper: return "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            case Category::Digits: return "0123456789";
            case Category::Symbols: return "!@#$%^&*()";
            default: return "";
        }
    }
    static const char *categoryName(Category kind) {
        switch (kind) {
            case Category::Lower: return "小写字母";
            case Category::Upper: return "大写字母";
            case Category::Digits: return "数字";
            case Category::Symbols: return "符号";
            case Category::ShowPassword: return "显示密码";
            case Category::Delete: return "删除";
            case Category::Connect: return "连接WiFi";
            case Category::Cancel: return "取消输入";
            default: return "";
        }
    }
    std::string categoryLabel() const {
        if (category() == Category::ShowPassword && showPassword_) return "隐藏密码";
        return categoryName(category());
    }
    char selectedCharacter() const {
        const auto chars = characters(category());
        return chars.empty() ? '\0' : chars[characterPositions_[category_] % chars.size()];
    }
    // The five neighboring characters wrap at both ends of a category.
    char neighboringCharacter(int relative) const {
        const auto chars = characters(category());
        if (chars.empty()) return '\0';
        const auto length = static_cast<int>(chars.size());
        const int selected = static_cast<int>(characterPositions_[category_]);
        return chars[static_cast<std::size_t>((selected + relative % length + length) % length)];
    }

    // In edit mode, all password characters are individual focusable items.
    // Two additional items follow: Save and Cancel. The selection wraps.
    const std::string &editPassword() const { return editPassword_; }
    std::size_t editPosition() const { return editPosition_; }
    std::size_t editItemCount() const { return editPassword_.size() + 2; }
    bool editSelectedCharacter() const { return editPosition_ < editPassword_.size(); }
    bool editSelectedSave() const { return editPosition_ == editPassword_.size(); }
    bool editSelectedCancel() const { return editPosition_ == editPassword_.size() + 1; }
    bool canUndo() const { return !undo_.empty(); }

    void press(Key key) {
        if (done()) return;
        error_.clear();
        if (stage_ == Stage::EditPassword) {
            editPress(key);
            return;
        }
        if (key == Key::Cancel) {
            if (stage_ == Stage::Characters) stage_ = Stage::Categories;
            else stage_ = Stage::Cancelled;
            return;
        }
        if (stage_ == Stage::Categories) {
            if (key == Key::Previous) category_ = (category_ + categoryCount - 1) % categoryCount;
            else if (key == Key::Next) category_ = (category_ + 1) % categoryCount;
            else if (key == Key::Confirm) {
                if (category_ < 4) stage_ = Stage::Characters;
                else if (category() == Category::ShowPassword) showPassword_ = !showPassword_;
                else if (category() == Category::Delete) enterEditor();
                else if (category() == Category::Connect) {
                    if (password_.size() < minimumLength_ || password_.size() > maximumLength_
                        || (legacyWep_ && password_.size() != 5 && password_.size() != 13)) {
                        error_ = "密码长度不正确";
                    } else stage_ = Stage::Submitted;
                } else stage_ = Stage::Cancelled;
            }
        } else if (stage_ == Stage::Characters) {
            const auto chars = characters(category());
            if (chars.empty()) return;
            auto &pos = characterPositions_[category_];
            if (key == Key::Previous) pos = (pos + chars.size() - 1) % chars.size();
            else if (key == Key::Next) pos = (pos + 1) % chars.size();
            else if (key == Key::Confirm) {
                if (password_.size() >= maximumLength_) error_ = "已达到密码上限";
                else password_.push_back(chars[pos]);
            }
        }
    }

    // Clear sensitive temporary data, including cancelled edit buffers/undo.
    void clearPassword() {
        wipe(password_);
        clearEditor();
    }

private:
    struct DeleteUndo { std::size_t position; char character; };
    static constexpr std::size_t categoryCount = static_cast<std::size_t>(Category::Count);

    static void wipe(std::string &text) {
        volatile char *bytes = text.empty() ? nullptr : &text[0];
        for (std::size_t i = 0; bytes && i < text.size(); ++i) bytes[i] = 0;
        text.clear();
    }
    void clearEditor() {
        wipe(editPassword_);
        for (auto &item : undo_) item.character = 0;
        undo_.clear();
        editPosition_ = 0;
    }
    void enterEditor() {
        clearEditor();
        editPassword_ = password_; // Changes are provisional until Save.
        stage_ = Stage::EditPassword;
    }
    void finishEditor(bool save) {
        if (save) {
            wipe(password_);
            password_ = std::move(editPassword_);
        }
        clearEditor();
        stage_ = Stage::Categories;
    }
    void editPress(Key key) {
        if (key == Key::Previous || key == Key::Next) {
            const auto count = editItemCount(); // Always at least Save + Cancel.
            editPosition_ = (editPosition_ + count + (key == Key::Previous ? count - 1 : 1)) % count;
        } else if (key == Key::Cancel) {
            // Returning from edit mode means undo ONE previous deletion.
            if (undo_.empty()) {
                error_ = "没有可撤销的删除";
            } else {
                const auto operation = undo_.back();
                undo_.pop_back();
                editPassword_.insert(editPassword_.begin() + operation.position, operation.character);
                editPosition_ = operation.position;
            }
        } else if (key == Key::Confirm) {
            if (editSelectedSave()) {
                finishEditor(true);
            } else if (editSelectedCancel()) {
                finishEditor(false);
            } else {
                const char removed = editPassword_[editPosition_];
                undo_.push_back({editPosition_, removed});
                editPassword_.erase(editPosition_, 1);
                // Focus next character or Save if the last character was removed.
                if (editPosition_ >= editPassword_.size()) editPosition_ = editPassword_.size();
            }
        }
    }

    Stage stage_ = Stage::Categories;
    std::size_t category_ = 0;
    std::array<std::size_t, categoryCount> characterPositions_{};
    std::string password_;
    std::string editPassword_;
    std::vector<DeleteUndo> undo_;
    std::size_t editPosition_ = 0;
    std::string error_;
    bool showPassword_ = false;
    std::size_t minimumLength_;
    std::size_t maximumLength_;
    bool legacyWep_;
};
