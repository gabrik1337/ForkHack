#include "Menu/Widgets.hpp"

#include "Menu/Binds.hpp"

#include "Utils/xorstr.h"

#include "Gfx/Blur.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

using namespace ImGui;

bool Menu::Keybind(const char* label, keybind_t* bind)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
    {
        ImGui::PopStyleColor();
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos;
    ImRect total_bb(pos, pos + ImVec2(256.0f * s, 36.0f * s));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        ImGui::PopStyleColor();
        return false;
    }

    uint32_t h = Hash_Label(label);
    item_animation_t& mod = item_animations[h];
    ImRect key_bb(pos + ImVec2(172.0f * s, 6.0f * s), pos + ImVec2(244.0f * s, 26.0f * s));
    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(key_bb, id, &hovered, &held);
    this->CreateAnimation(mod.hovered_alpha, hovered, 1.f, AnimLerp);

    static std::map<uint32_t, int> capture;
    int& state = capture[h];
    bool changed = false;

    if (pressed && state == 0)
    {
        state = 1;
    }

    if (state == 1)
    {
        if (!AnyKeyDown())
        {
            state = 2;
        }
    }
    else if (state == 2)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            bind->key = -1;
            state = 0;
            changed = true;
        }
        else
        {
            int found_key = PollPressedKey();

            if (found_key >= 0)
            {
                bind->key = found_key;
                state = 0;
                changed = true;
            }
        }
    }

    float back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);
    draw_list->AddRectFilled(pos, pos + ImVec2(256.0f * s, 32.0f * s), back_clr.as_imcolor(), 4.f * s);

    if (label_size.x > 0.0f)
        RenderText(ImVec2(pos.x + 12.0f * s, pos.y + 7.0f * s), label);

    static const std::string dots_s = "...";
    const char* shown = state > 0 ? dots_s.c_str() : KeyName(bind->key);
    ImVec2 shown_size = CalcTextSize(shown);
    draw_list->AddRectFilled(key_bb.Min, key_bb.Max, c_color(0, 0, 0, 80 * alpha).as_imcolor(), 3.f * s);
    RenderTextClipped(key_bb.Min, key_bb.Max, shown, NULL, &shown_size, ImVec2(0.5f, 0.5f));

    ImGui::PopStyleColor();
    return changed;
}
