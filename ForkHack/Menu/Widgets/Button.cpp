#include "Menu/Widgets.hpp"

#include "Menu/Binds.hpp"

#include "Utils/xorstr.h"

#include "Gfx/Blur.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace ImGui;

bool Menu::Button(const char* label)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        PopStyleColor();

        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(256.0f * s, 36.0f * s));
    ItemSize(bb, style.FramePadding.y);

    if (!ItemAdd(bb, id))
    {
        PopStyleColor();

        return false;
    }

    const ImRect rect(pos, pos + ImVec2(256.0f * s, 32.0f * s));
    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held);
    item_animation_t& mod = item_animations[Hash_Label(label) + Hash_Label("btn")];
    CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);

    float back_alpha = (20 + 30 * mod.hovered_alpha + 50 * (hovered && held)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);
    draw_list->AddRectFilled(rect.Min, rect.Max, back_clr.as_imcolor(), 4.0f * s);
    RenderTextClipped(rect.Min, rect.Max, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    return pressed;
}
