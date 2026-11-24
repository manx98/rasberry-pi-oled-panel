#include "panel/wifi_password_state.h"
#include <cassert>
#include <string>

using S = WifiPasswordState;

void gotoCategory(S &keyboard, S::Category target) {
    for (int tries = 0; keyboard.category() != target && tries < 9; ++tries)
        keyboard.press(S::Key::Next);
    assert(keyboard.category() == target);
}
void addChar(S &keyboard, S::Category category, char c) {
    gotoCategory(keyboard, category);
    keyboard.press(S::Key::Confirm);
    for (int i = 0; i < 30 && keyboard.selectedCharacter() != c; ++i)
        keyboard.press(S::Key::Next);
    assert(keyboard.selectedCharacter() == c);
    keyboard.press(S::Key::Confirm);
    keyboard.press(S::Key::Cancel);
}

int main() {
    assert(S::characters(S::Category::Lower) == "abcdefghijklmnopqrstuvwxyz");
    assert(S::characters(S::Category::Upper) == "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    assert(S::characters(S::Category::Digits) == "0123456789");
    assert(S::characters(S::Category::Symbols) == "!@#$%^&*()");
    {
        S keyboard;
        keyboard.press(S::Key::Cancel);
        assert(keyboard.done() && !keyboard.submitted());
        assert(keyboard.password().empty());
    }
    {
        S keyboard;
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(!keyboard.done() && !keyboard.error().empty());
        gotoCategory(keyboard, S::Category::Lower);
        for (int i = 0; i < 4; ++i) addChar(keyboard, S::Category::Lower, 'a');
        addChar(keyboard, S::Category::Upper, 'Z');
        addChar(keyboard, S::Category::Digits, '9');
        addChar(keyboard, S::Category::Symbols, '!');
        addChar(keyboard, S::Category::Symbols, ')');
        assert(keyboard.password() == "aaaaZ9!)");
        assert(keyboard.masked() == "********");
        assert(keyboard.previewPassword() == "********");

        gotoCategory(keyboard, S::Category::ShowPassword);
        assert(keyboard.categoryLabel() == "显示密码");
        keyboard.press(S::Key::Confirm);
        assert(keyboard.showPassword());
        assert(keyboard.categoryLabel() == "隐藏密码");
        assert(keyboard.previewPassword() == "aaaaZ9!)");
        keyboard.press(S::Key::Confirm);
        assert(!keyboard.showPassword());
        assert(keyboard.previewPassword() == "********");

        // Edit middle characters and cancel. Original password is preserved.
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.stage() == S::Stage::EditPassword);
        assert(keyboard.editPassword() == "aaaaZ9!)");
        assert(keyboard.editPosition() == 0);
        keyboard.press(S::Key::Next);
        keyboard.press(S::Key::Next);
        assert(keyboard.editPosition() == 2);
        keyboard.press(S::Key::Confirm); // Delete third 'a'.
        assert(keyboard.editPassword() == "aaaZ9!)");
        assert(keyboard.password() == "aaaaZ9!)");
        assert(keyboard.canUndo());
        keyboard.press(S::Key::Cancel); // Undo exactly one deletion.
        assert(keyboard.editPassword() == "aaaaZ9!)");
        assert(keyboard.editPosition() == 2);
        assert(!keyboard.canUndo());
        keyboard.press(S::Key::Cancel);
        assert(keyboard.editPassword() == "aaaaZ9!)");
        assert(!keyboard.error().empty());

        keyboard.press(S::Key::Confirm); // Delete third 'a' again.
        keyboard.press(S::Key::Confirm); // Delete 'a' again, same index.
        assert(keyboard.editPassword() == "aaZ9!)");
        keyboard.press(S::Key::Cancel); // Last deletion only.
        assert(keyboard.editPassword() == "aaaZ9!)");
        assert(keyboard.canUndo());
        // Wrap left across the character list and both actions.
        keyboard.press(S::Key::Previous);
        keyboard.press(S::Key::Previous);
        keyboard.press(S::Key::Previous);
        assert(keyboard.editSelectedCancel());
        keyboard.press(S::Key::Confirm); // Cancel ALL unsaved changes.
        assert(keyboard.stage() == S::Stage::Categories);
        assert(keyboard.password() == "aaaaZ9!)");

        // Delete a selected internal character and save that edit.
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        for (int i = 0; i < 4; ++i) keyboard.press(S::Key::Next);
        assert(keyboard.editPassword()[keyboard.editPosition()] == 'Z');
        keyboard.press(S::Key::Confirm);
        assert(keyboard.editPassword() == "aaaa9!)");
        const auto length = keyboard.editPassword().size();
        // Selecting Save is one more slot after the last character.
        for (int i = 0; i < 30 && !keyboard.editSelectedSave(); ++i)
            keyboard.press(S::Key::Next);
        assert(keyboard.editPosition() == length);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.stage() == S::Stage::Categories);
        assert(keyboard.password() == "aaaa9!)");
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(!keyboard.done() && !keyboard.error().empty());
        addChar(keyboard, S::Category::Symbols, ')'); // Can still input.
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.done() && keyboard.submitted());
        keyboard.press(S::Key::Cancel); // Ignore keys after completion.
        assert(keyboard.submitted());
        keyboard.clearPassword();
        assert(keyboard.password().empty());
    }
    {
        S keyboard(5, 13, true); // WEP accepts only 5 or 13 characters.
        for (int i = 0; i < 6; ++i) addChar(keyboard, S::Category::Lower, 'x');
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(!keyboard.done());
        assert(!keyboard.error().empty());
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        // Delete one character, then press Save.
        keyboard.press(S::Key::Confirm);
        assert(keyboard.editPassword().size() == 5);
        for (int i = 0; i < 15 && !keyboard.editSelectedSave(); ++i)
            keyboard.press(S::Key::Next);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.password().size() == 5);
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.submitted());
    }
    {
        // Empty editor still has Save and Cancel; wrap and cancel safely.
        S keyboard;
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.stage() == S::Stage::EditPassword);
        assert(keyboard.editSelectedSave());
        keyboard.press(S::Key::Previous);
        assert(keyboard.editSelectedCancel());
        keyboard.press(S::Key::Confirm);
        assert(keyboard.stage() == S::Stage::Categories);
        assert(keyboard.password().empty());
    }
    {
        // Long WPA passwords remain fully editable via character-position
        // navigation; deleting the final character and undo work at length 63.
        S keyboard;
        keyboard.press(S::Key::Confirm); // Lowercase characters.
        for (int i = 0; i < 63; ++i) keyboard.press(S::Key::Confirm);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.password().size() == 63);
        assert(!keyboard.error().empty()); // Upper bound enforced.
        keyboard.press(S::Key::Cancel);
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.editPassword().size() == 63);
        keyboard.press(S::Key::Previous); // Cancel
        keyboard.press(S::Key::Previous); // Save
        keyboard.press(S::Key::Previous); // Last character
        assert(keyboard.editPosition() == 62);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.editPassword().size() == 62);
        assert(keyboard.editSelectedSave());
        keyboard.press(S::Key::Cancel);
        assert(keyboard.editPassword().size() == 63);
        assert(keyboard.editPosition() == 62);
        keyboard.press(S::Key::Next);
        assert(keyboard.editSelectedSave());
        keyboard.press(S::Key::Confirm);
        assert(keyboard.password().size() == 63);
        keyboard.clearPassword();
        assert(keyboard.password().empty());
    }
    {
        S keyboard(0, 2);
        keyboard.press(S::Key::Previous);
        assert(keyboard.category() == S::Category::Cancel); // Category wrap.
        keyboard.press(S::Key::Next);
        assert(keyboard.category() == S::Category::Lower);
        keyboard.press(S::Key::Confirm);
        keyboard.press(S::Key::Previous);
        assert(keyboard.selectedCharacter() == 'z'); // Character wrap.
        assert(keyboard.neighboringCharacter(-1) == 'y');
        assert(keyboard.neighboringCharacter(1) == 'a');
        keyboard.press(S::Key::Confirm);
        keyboard.press(S::Key::Confirm);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.password() == "zz");
        assert(!keyboard.error().empty());
        keyboard.press(S::Key::Cancel);
        gotoCategory(keyboard, S::Category::Delete);
        keyboard.press(S::Key::Confirm);
        // Edit navigation cycles: first -> Cancel -> Save -> last char.
        keyboard.press(S::Key::Previous);
        assert(keyboard.editSelectedCancel());
        keyboard.press(S::Key::Previous);
        assert(keyboard.editSelectedSave());
        keyboard.press(S::Key::Previous);
        assert(keyboard.editSelectedCharacter() && keyboard.editPosition() == 1);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.editPassword() == "z");
        keyboard.press(S::Key::Cancel);
        assert(keyboard.editPassword() == "zz");
        keyboard.press(S::Key::Next);
        keyboard.press(S::Key::Next); // Go to Cancel from char 1.
        assert(keyboard.editSelectedCancel());
        keyboard.press(S::Key::Confirm);
        assert(keyboard.password() == "zz");
        gotoCategory(keyboard, S::Category::Connect);
        keyboard.press(S::Key::Confirm);
        assert(keyboard.submitted());
    }
}
