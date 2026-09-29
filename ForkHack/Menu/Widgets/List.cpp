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

bool Menu::Listbox(const char* label, int* current, const char* const items[], int count, int height_in_items)
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
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    float row_h = g.FontSize * s + style.ItemSpacing.y;
    ImVec2 box_size = ImVec2(256.0f * s, row_h * height_in_items + 8.0f * s);
    ImVec2 pos = window->DC.CursorPos;
    ImRect total_bb(pos, pos + ImVec2(256.0f * s, label_size.x > 0.0f ? 22.0f * s + box_size.y : box_size.y));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        ImGui::PopStyleColor();
        return false;
    }

    if (label_size.x > 0.0f)
        RenderText(ImVec2(pos.x + 12.0f * s, pos.y), label);

    ImVec2 box_min = pos + ImVec2(0, label_size.x > 0.0f ? 22.0f * s : 0);
    ImVec2 box_max = box_min + box_size;
    draw_list->AddRectFilled(box_min, box_max, c_color(217, 217, 217).new_alpha(20 * alpha).as_imcolor(), 4.f * s);
    draw_list->AddRect(box_min, box_max, c_color(0, 0, 0, 80 * alpha).as_imcolor(), 4.f * s);

    bool changed = false;
    ImGui::SetCursorScreenPos(box_min + ImVec2(6.0f * s, 4.0f * s));
    std::string child_id = std::string("##") + label + "_list";
    ImGui::BeginChild(child_id.c_str(), box_size - ImVec2(12.0f * s, 8.0f * s), false, ImGuiWindowFlags_NoSavedSettings);
    for (int i = 0; i < count; i++)
    {
        bool selected = (*current == i);
        if (Selectable(items[i], selected, 1.f))
        {
            *current = i;
            changed = true;
        }
    }
    ImGui::EndChild(false);
    ImGui::SetCursorScreenPos(pos + ImVec2(0, total_bb.Max.y - total_bb.Min.y));

    ImGui::PopStyleColor();
    return changed;
}

static bool Equals(std::string a, std::string b)
{
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    return std::strstr(a.c_str(), b.c_str());
}

bool Menu::ListboxSelectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg, int iter)
{
    float alpha = GetAlpha();
    const float s = GetScale();

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
        PushColumnsBackground();

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(220.0f * s, 36.0f * s);
    ImVec2 pos = window->DC.CursorPos;
    pos.x += 6.0f * s;
    ImRect bb_inner(pos, pos + size);
    bb_inner.Min.x += 11.0f * s;
    bb_inner.Max.x += 11.0f * s;

    bb_inner.Min.y += 8.0f * s;
    bb_inner.Max.y += 8.0f * s;

    ItemSize(size, 0.0f);

    ImVec2 window_padding = window->WindowPadding;
    float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
    ImRect bb(pos, pos + size);
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
        bb.Max.x += window_padding.x;

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
            PopColumnsBackground();
        return false;
    }

    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID)
        button_flags |= ImGuiButtonFlags_NoHoldingActiveID;
    if (flags & ImGuiSelectableFlags_PressedOnClick)
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    if (flags & ImGuiSelectableFlags_PressedOnRelease)
        button_flags |= ImGuiButtonFlags_PressedOnRelease;
    if (flags & ImGuiSelectableFlags_Disabled)
        button_flags |= ImGuiButtonFlags_Disabled;
    if (flags & ImGuiSelectableFlags_AllowDoubleClick)
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    item_animation_t& mod = item_animations[Hash_Label(label) + (uint32_t)iter];

    if (alpha_pass > 0.0f)
    {
        CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);
        CreateAnimation(mod.alpha, pressed, 0.6f, AnimLerp);
    }

    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            g.NavDisableHighlight = true;
            SetNavID(id, window->DC.NavLayerCurrent);
        }
    }
    if (pressed)
        MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        SetItemAllowOverlap();

    if (selected != was_selected)
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
    {
        PopColumnsBackground();
        bb.Max.x -= (GetContentRegionMax().x - max_x);
    }

    if (alpha_pass > 0.0f)
    {
        float text_clr = selected ? 1.0f : 0.58f + 0.42f * mod.hovered_alpha;
        float back_alpha = (20 + (30 * mod.hovered_alpha) + (50 * (hovered && held))) * alpha;
        c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);

        window->DrawList->AddRectFilled(bb.Min, bb.Max - ImVec2(0.0f, 4.0f * s), back_clr.as_imcolor(), 4.0f * s);

        PushStyleColor(ImGuiCol_Text, ImVec4(text_clr, text_clr, text_clr, text_clr * alpha * alpha_pass));
        RenderTextClipped(bb_inner.Min, bb_inner.Max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
        PopStyleColor();
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    return pressed;
}

bool Menu::ListBoxHeader(const char* label, const ImVec2& size_arg)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f * alpha));

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 size = CalcItemSize(size_arg, 256.0f * s, GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
    ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
    ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    window->DC.LastItemRect = bb;
    g.NextItemData.ClearFlags();

    if (!IsRectVisible(bb.Min, bb.Max))
    {
        ItemSize(bb.GetSize(), style.FramePadding.y);
        ItemAdd(bb, 0, &frame_bb);
        return false;
    }

    BeginGroup();

    float back_alpha = 20 * alpha;
    c_color back_clr = c_color(217, 217, 217).new_alpha((int)back_alpha);

    PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    PushStyleColor(ImGuiCol_ChildBg, back_clr.as_imvec4());

    BeginChild(id, size, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

    PopStyleColor(2);
    PopStyleVar();

    return true;
}

bool Menu::ListBoxHeaderStart(const char* label, int items_count, int height_in_items)
{
    if (height_in_items < 0)
        height_in_items = ImMin(items_count, 7);
    const ImGuiStyle& style = GetStyle();
    float height_in_items_f = (height_in_items < items_count) ? (height_in_items + 0.25f) : (height_in_items + 0.00f);

    ImVec2 size;
    size.x = 0.0f;
    size.y = ImFloor(GetTextLineHeightWithSpacing() * height_in_items_f + style.FramePadding.y * 2.0f);
    return ListBoxHeader(label, size);
}

void Menu::ListBoxFooter()
{
    ImGuiWindow* parent_window = GetCurrentWindow()->ParentWindow;
    const ImRect bb = parent_window->DC.LastItemRect;
    const ImGuiStyle& style = GetStyle();

    EndChildFrame();

    SameLine();
    parent_window->DC.CursorPos = bb.Min;
    ItemSize(bb, style.FramePadding.y);
    EndGroup();
}

bool Menu::ListBoxWrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items, const std::string& compare_text)
{
    if (!ListBoxHeaderStart(label, items_count, height_in_items))
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;

    std::string lower_string = compare_text;
    std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), ::tolower);

    std::vector<std::pair<std::string, int>> filtered_elements{};
    if (lower_string.size() > 0)
    {
        for (int i = 0; i < items_count; ++i)
        {
            const char* item_text = nullptr;

            if (!items_getter(data, i, &item_text))
                item_text = "*Unknown item*";

            if (Equals(item_text, lower_string))
                filtered_elements.emplace_back(std::make_pair(item_text, i));
        }
    }

    const int filtered_count = (int)filtered_elements.size();

    ImGuiListClipper clipper(filtered_count > 0 ? filtered_count : items_count, GetTextLineHeightWithSpacing());
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            PushID(i);

            const char* item_text = nullptr;

            if (!items_getter(data, i, &item_text))
                item_text = "*Unknown item*";

            int index = filtered_count > 0 ? filtered_elements[i].second : i;
            const char* name = filtered_count > 0 ? filtered_elements[i].first.c_str() : item_text;

            const bool item_selected = (index == *current_item);
            if (ListboxSelectable(name, item_selected, 1.0f, 0, ImVec2(0, 0), index))
            {
                *current_item = index;
                value_changed = true;
            }

            PopID();
        }
    }

    ListBoxFooter();
    if (value_changed)
        MarkItemEdited(g.CurrentWindow->DC.LastItemId);

    return value_changed;
}

bool Menu::Listbox(const char* label, int* current_item, std::vector<std::string>& values, int height_in_items, const std::string& compare_text)
{
    if (values.empty())
    {
        return false;
    }

    return ListBoxWrapper(label, current_item, Vector_Getter, static_cast<void*>(&values), (int)values.size(), height_in_items, compare_text);
}
