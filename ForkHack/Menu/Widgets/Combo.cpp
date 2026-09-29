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

bool Menu::Selectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    float alpha = GetAlpha();

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
    {
        PushColumnsBackground();
    }

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb_inner(pos, pos + size);
    ItemSize(size, 0.0f);

    ImVec2 window_padding = window->WindowPadding;
    float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
    ImVec2 size_draw((size_arg.x != 0 && !(flags & ImGuiSelectableFlags_DrawFillAvailWidth)) ? size_arg.x : w_draw, size_arg.y != 0.0f ? size_arg.y : size.y);
    ImRect bb(pos, pos + size_draw);

    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
    {
        bb.Max.x += window_padding.x;
    }

    bool item_add;

    if (flags & ImGuiSelectableFlags_Disabled)
    {
        ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
        window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add = ItemAdd(bb, id);
        window->DC.ItemFlags = backup_item_flags;
    }
    else
    {
        item_add = ItemAdd(bb, id);
    }

    if (!item_add)
    {
        if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
        {
            PopColumnsBackground();
        }

        return false;
    }

    ImGuiButtonFlags button_flags = 0;

    if (flags & ImGuiSelectableFlags_NoHoldingActiveID)
    {
        button_flags |= ImGuiButtonFlags_NoHoldingActiveID;
    }

    if (flags & ImGuiSelectableFlags_PressedOnClick)
    {
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    }

    if (flags & ImGuiSelectableFlags_PressedOnRelease)
    {
        button_flags |= ImGuiButtonFlags_PressedOnRelease;
    }

    if (flags & ImGuiSelectableFlags_Disabled)
    {
        button_flags |= ImGuiButtonFlags_Disabled;
    }

    if (flags & ImGuiSelectableFlags_AllowDoubleClick)
    {
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    }

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
    {
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    }

    if (flags & ImGuiSelectableFlags_Disabled)
    {
        selected = false;
    }

    const bool was_selected = selected;
    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
    item_animation_t& mod = item_animations[Hash_Label(label) + Hash_Label("_sel")];
    CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);
    CreateAnimation(mod.alpha, pressed, 0.6f, AnimLerp);

    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            g.NavDisableHighlight = true;
            SetNavID(id, window->DC.NavLayerCurrent);
        }
    }

    if (pressed)
    {
        MarkItemEdited(id);
    }

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
    {
        SetItemAllowOverlap();
    }

    if (selected != was_selected)
    {
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;
    }

    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
    {
        hovered = true;
    }

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
    {
        PopColumnsBackground();
        bb.Max.x -= (GetContentRegionMax().x - max_x);
    }

    float text_clr = selected ? 1.0f : 0.58f + 0.42f * mod.hovered_alpha;
    PushStyleColor(ImGuiCol_Text, ImVec4(text_clr, text_clr, text_clr, text_clr * alpha * alpha_pass));
    RenderTextClipped(bb_inner.Min, bb_inner.Max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    PopStyleColor();

    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
    {
        CloseCurrentPopup();
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);

    return pressed;
}

bool Menu::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags, int item_cnt)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * alpha));

    ImGuiContext& g = *GImGui;
    bool has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
    g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = 256.0f * s;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, 48.0f * s));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.0f, 4.0f * s));
    ItemSize(total_bb, style.FramePadding.y);

    if (!ItemAdd(total_bb, id, &frame_bb))
    {
        return false;
    }

    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
    bool popup_open = IsPopupOpen(id);
    item_animation_t& mod = item_animations[Hash_Label(label)];
    CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);

    float back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);
    draw_list->AddRectFilled(frame_bb.Min, frame_bb.Max, back_clr.u32(), 4.0f * s);

    if (label_size.x > 0)
    {
        RenderText(ImVec2(frame_bb.Min.x + 12.0f * s, frame_bb.Min.y + 16.0f * s), label);
    }

    if ((pressed || g.NavActivateId == id) && !popup_open)
    {
        if (window->DC.NavLayerCurrent == 0)
        {
            window->NavLastIds[0] = id;
        }

        OpenPopupEx(id);
        popup_open = true;
    }

    mod.active = popup_open;
    CreateAnimation(mod.alpha, popup_open, 0.5f, AnimLerp);

    ImVec2 bg_size = ImVec2(123.0f * s, 31.0f * s);

    if (preview_value != NULL)
    {
        ImVec2 size = CalcTextSize(preview_value);
        float cur_pos = size.x + 36.0f * s;
        float out_pos = cur_pos + ((bg_size.x - cur_pos)) * mod.alpha;
        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
        RenderTextClipped(frame_bb.Max - ImVec2(out_pos, 32.0f * s), frame_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
        PopStyleColor();
    }

    ImVec2 combo_bg_min = frame_bb.Min + ImVec2(126.0f * s, 9.0f * s);
    ImVec2 combo_bg_max = combo_bg_min + bg_size;

    if (popup_open)
    {
        draw_list->AddRectFilled(combo_bg_min, combo_bg_max, c_color(5, 5, 5, (int)(alpha * mod.alpha)).as_imcolor(), 4.0f * s, ImDrawCornerFlags_Top);
    }

    if (popup_open)
    {
        draw_list->AddLine(combo_bg_max - ImVec2(bg_size.x, 1.0f * s), combo_bg_max - ImVec2(0, 1.0f * s), c_color(255, 255, 255, (int)(12.75f * alpha * mod.alpha)).u32(), 1.0f * s);
    }

    const ImVec2 left_line_min = frame_bb.Min + ImVec2(232.0f * s, 22.0f * s);
    const ImVec2 left_line_max = frame_bb.Min + ImVec2(236.0f * s, 26.0f * s);
    draw_list->AddLine(left_line_min, left_line_max, c_color(255, 255, 255, (int)(255 * alpha)).u32(), 1.5f * s);
    draw_list->AddLine(left_line_min + ImVec2(9.0f * s, -1.0f * s), left_line_max, c_color(255, 255, 255, (int)(255 * alpha)).u32(), 1.5f * s);

    if (!popup_open)
    {
        return false;
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size);
    SetNextWindowPos(frame_bb.Min + ImVec2(126.0f * s, 40.0f * s));

    float max_size = std::clamp(Calc_Max_Popup_Height(item_cnt), 0.0f, 200.0f * s);
    ImVec2 items_size = ImVec2(123.0f * s, max_size * mod.alpha);
    SetNextWindowSize(items_size);
    SetNextWindowBgAlpha(alpha * mod.alpha);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7.0f * s, style.WindowPadding.y * s));
    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar();

    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);

        return false;
    }

    PopStyleColor();

    return true;
}

static std::string Truncate_Chars(const std::string& s, size_t max_chars)
{
    size_t i = 0;
    size_t chars = 0;

    while (i < s.size() && chars < max_chars)
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = 1;

        if ((c & 0x80) == 0)
        {
            len = 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            len = 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            len = 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            len = 4;
        }
        else
        {
            i++;
            chars++;
            continue;
        }

        if (i + len > s.size())
        {
            break;
        }

        i += len;
        chars++;
    }

    if (i < s.size())
    {
        return s.substr(0, i) + "...";
    }

    return s;
}

static const char* Select_Text()
{
    static const std::string ru = "Выбрать";
    static const std::string en = "Select";
    return (g_cfg.language == 1 ? en : ru).c_str();
}

bool Menu::ComboWrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;
    const char* preview_value = NULL;

    if (*current_item >= 0 && *current_item < items_count)
    {
        items_getter(data, *current_item, &preview_value);
    }
    else
    {
        preview_value = Select_Text();
    }

    std::string str = Truncate_Chars(std::string(preview_value), 15);

    if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
    {
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, Calc_Max_Popup_Height(popup_max_height_in_items)));
    }

    if (!BeginCombo(label, str.c_str(), ImGuiComboFlags_None, items_count))
    {
        return false;
    }

    item_animation_t& mod = item_animations[Hash_Label(label)];
    bool value_changed = false;

    for (int i = 0; i < items_count; i++)
    {
        PushID((void*)(intptr_t)i);
        const bool item_selected = (i == *current_item);
        const char* item_text = nullptr;
        static const std::string unknown_s = "item?";

        if (!items_getter(data, i, &item_text))
        {
            item_text = unknown_s.c_str();
        }

        if (Selectable(item_text, item_selected, mod.alpha))
        {
            value_changed = true;
            *current_item = i;
        }

        PopID();
    }

    EndCombo();

    return value_changed;
}

bool Menu::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    return ComboWrapper(label, current_item, Items_Array_Getter, (void*)items, items_count, height_in_items);
}

bool Menu::Selectable2(const char* label, bool* selected, ImGuiSelectableFlags flags, const ImVec2& size_arg, float alpha_pass)
{
    if (Selectable(label, *selected, alpha_pass, flags, size_arg))
    {
        *selected = !*selected;

        return true;
    }

    return false;
}

bool Menu::SelectableFlags(const char* label, unsigned int* flags, unsigned int flags_value, float alpha_pass)
{
    bool v = ((*flags & flags_value) == flags_value);
    bool pressed = Selectable2(label, &v, ImGuiSelectableFlags_DontClosePopups, ImVec2(0, 0), alpha_pass);

    if (pressed)
    {
        if (v)
        {
            *flags |= flags_value;
        }
        else
        {
            *flags &= ~flags_value;
        }
    }

    return pressed;
}

void Menu::MultiCombo(const char* label, unsigned int& var, std::vector<std::string> elements)
{
    int t = 0;
    std::string& items = combo_items[label];
    items.clear();

    for (int i = 0; i < (int)elements.size(); i++)
    {
        if (var & (1 << i))
        {
            if (t++ > 0)
            {
                items += ", ";
            }

            items += elements[i];
        }
    }

    if (items.length() >= 12)
    {
        items = Truncate_Chars(items, 12);
    }

    if (BeginCombo(label, items.empty() ? Select_Text() : items.c_str(), 0, (int)elements.size()))
    {
        item_animation_t& mod = item_animations[Hash_Label(label)];

        for (int i = 0; i < (int)elements.size(); i++)
        {
            SelectableFlags(elements[i].c_str(), &var, (1 << i), mod.alpha);
        }

        EndCombo();
    }
}
