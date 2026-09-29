#include "Menu/Binds.hpp"

#include "Utils/xorstr.h"

#include "Core/Config.hpp"

#include <windows.h>

namespace
{
    std::vector<key_entry_t> BuildKeyList()
    {
        std::vector<key_entry_t> list;

        auto add = [&](int vk, const char* ru, const char* en)
        {
            list.push_back({ vk, ru, en });
        };

        add(VK_LSHIFT, "Левый Shift", "LShift");
        add(VK_RSHIFT, "Правый Shift", "RShift");
        add(VK_LCONTROL, "Левый Ctrl", "LCtrl");
        add(VK_RCONTROL, "Правый Ctrl", "RCtrl");
        add(VK_LMENU, "Левый Alt", "LAlt");
        add(VK_RMENU, "Правый Alt", "RAlt");

        add(VK_LBUTTON, "ЛКМ", "M1");
        add(VK_RBUTTON, "ПКМ", "M2");
        add(VK_MBUTTON, "СКМ", "M3");
        add(VK_XBUTTON1, "Мышь 4", "M4");
        add(VK_XBUTTON2, "Мышь 5", "M5");

        add(VK_SHIFT, "Шифт", "Shift");
        add(VK_CONTROL, "Контрол", "Ctrl");
        add(VK_MENU, "Альт", "Alt");

        add(VK_BACK, "Backspace", "Backspace");
        add(VK_TAB, "Tab", "Tab");
        add(VK_RETURN, "Enter", "Enter");
        add(VK_CAPITAL, "Caps Lock", "Caps Lock");
        add(VK_SPACE, "Пробел", "Space");
        add(VK_PRIOR, "Page Up", "Page Up");
        add(VK_NEXT, "Page Down", "Page Down");
        add(VK_END, "End", "End");
        add(VK_HOME, "Home", "Home");
        add(VK_LEFT, "Стрелка влево", "Left");
        add(VK_UP, "Стрелка вверх", "Up");
        add(VK_RIGHT, "Стрелка вправо", "Right");
        add(VK_DOWN, "Стрелка вниз", "Down");
        add(VK_INSERT, "Insert", "Insert");
        add(VK_DELETE, "Delete", "Delete");

        for (char c = '0'; c <= '9'; c++)
        {
            char s[2] = { c, '\0' };
            add(c, s, s);
        }

        for (char c = 'A'; c <= 'Z'; c++)
        {
            char s[2] = { c, '\0' };
            add(c, s, s);
        }

        for (int i = 0; i <= 9; i++)
        {
            char s[8]{};
            s[0] = 'N';
            s[1] = 'u';
            s[2] = 'm';
            s[3] = ' ';
            s[4] = (char)('0' + i);
            add(VK_NUMPAD0 + i, s, s);
        }

        add(VK_MULTIPLY, "Num *", "Num *");
        add(VK_ADD, "Num +", "Num +");
        add(VK_SUBTRACT, "Num -", "Num -");
        add(VK_DECIMAL, "Num .", "Num .");
        add(VK_DIVIDE, "Num /", "Num /");

        for (int i = 1; i <= 12; i++)
        {
            char s[8]{};
            s[0] = 'F';
            s[1] = (char)('0' + i / 10);
            s[2] = (char)('0' + i % 10);

            if (s[1] == '0')
            {
                s[1] = s[2];
                s[2] = '\0';
            }

            add(VK_F1 + i - 1, s, s);
        }

        add(VK_OEM_MINUS, "-", "-");
        add(VK_OEM_PLUS, "=", "=");
        add(VK_OEM_4, "[", "[");
        add(VK_OEM_6, "]", "]");
        add(VK_OEM_1, ";", ";");
        add(VK_OEM_7, "'", "'");
        add(VK_OEM_3, "`", "`");
        add(VK_OEM_5, "\\", "\\");
        add(VK_OEM_COMMA, ",", ",");
        add(VK_OEM_PERIOD, ".", ".");
        add(VK_OEM_2, "/", "/");

        return list;
    }
}

const std::vector<key_entry_t>& GetKeyList()
{
    static const std::vector<key_entry_t> list = BuildKeyList();
    return list;
}

bool AnyKeyDown()
{
    for (const auto& e : GetKeyList())
    {
        if ((GetAsyncKeyState(e.vk) & 0x8000) != 0)
        {
            return true;
        }
    }

    return false;
}

int PollPressedKey()
{
    for (const auto& e : GetKeyList())
    {
        if ((GetAsyncKeyState(e.vk) & 0x8000) != 0)
        {
            return e.vk;
        }
    }

    return -1;
}

const char* KeyName(int key)
{
    if (key < 0)
    {
        static const std::string none_ru = "Нет";
        static const std::string none_en = "None";
        return (g_cfg.language == 1 ? none_en : none_ru).c_str();
    }

    for (const auto& e : GetKeyList())
    {
        if (e.vk == key)
        {
            return (g_cfg.language == 1 ? e.en : e.ru).c_str();
        }
    }

    static const std::string unknown = "Key";
    return unknown.c_str();
}
