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

static const char* Fix_Float_Format(const char* fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0)
    {
        return "%d";
    }

    const char* fmt_start = ImParseFormatFindStart(fmt);
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);

    if (fmt_end > fmt_start && fmt_end[-1] == 'f')
    {
        if (fmt_start == fmt && fmt_end[0] == 0)
        {
            return "%d";
        }

        ImGuiContext& g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end);

        return g.TempBuffer;
    }

    return fmt;
}

bool Menu::SliderScalar(const char* label, ImGuiDataType data_type, void* data, const void* min, const void* max, const char* format, float power)
{
    float alpha = GetAlpha();
    const float s = GetScale();

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = 256.0f * s;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, 56.0f * s));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.0f, 4.0f * s));
    ItemSize(total_bb, style.FramePadding.y);

    if (!ItemAdd(total_bb, id, &frame_bb))
    {
        return false;
    }

    if (format == NULL)
    {
        format = DataTypeGetInfo(data_type)->PrintFmt;
    }
    else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
    {
        format = Fix_Float_Format(format);
    }

    item_animation_t& mod_frame = item_animations[Hash_Label(label) + Hash_Label("frame")];
    item_animation_t& mod = item_animations[Hash_Label(label)];
    const bool hovered_frame = ItemHoverable(frame_bb, id);
    CreateAnimation(mod_frame.hovered_alpha, hovered_frame, 1.0f, AnimLerp);
    CreateAnimation(mod_frame.alpha, true, 0.3f, AnimSkipDisable | AnimLerp);

    ImVec2 slider_start = frame_bb.Min + ImVec2(12.0f * s, 28.0f * s);
    ImVec2 slider_size = ImVec2(232.0f * s, 24.0f * s);
    ImRect slider_bounds(slider_start, slider_start + slider_size);
    const bool hovered = ItemHoverable(slider_bounds, id);
    const bool focus_requested = FocusableItemRegister(window, id);
    const bool clicked = (hovered && g.IO.MouseClicked[0]);

    if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);

        if (focus_requested || g.NavInputId == id)
        {
            FocusableItemUnregister(window);
        }
    }

    float back_alpha = (20 + (30 * mod_frame.hovered_alpha)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod_frame.hovered_alpha)).new_alpha((int)back_alpha);
    draw_list->AddRectFilled(frame_bb.Min, frame_bb.Max, back_clr.as_imcolor(), 4.0f * s);

    ImRect render_bounds = ImRect(slider_bounds.Min + ImVec2(0.0f, 9.0f * s), slider_bounds.Max - ImVec2(0.0f, 9.0f * s));
    draw_list->AddRectFilled(render_bounds.Min, render_bounds.Max, c_color(0, 0, 0, (int)(80 * alpha)).as_imcolor(), 3.0f * s);

    ImRect grab_bb;
    bool value_changed = SliderBehavior(slider_bounds, id, data_type, data, min, max, format, power, ImGuiSliderFlags_IgnoreGrabCalc, &grab_bb);

    if (value_changed)
    {
        MarkItemEdited(id);
    }

    if (grab_bb.Max.x > grab_bb.Min.x)
    {
        c_color clr = g_cfg.accent.to_color();
        ImRect render_grab_bb = ImRect(grab_bb.Min + ImVec2(0.0f, 9.0f * s), grab_bb.Max - ImVec2(0.0f, 9.0f * s));

        auto gradient = ImDrawPaint().SetLinearGradient(render_bounds.Min, render_grab_bb.Max + ImVec2(0.0f, 2.0f * s), clr.new_alpha((int)(clr.a() * alpha * 0.5f)).as_imvec4(), clr.new_alpha((int)(clr.a() * alpha)).as_imvec4());

        float bb_nomove = render_grab_bb.Max.x - 20.0f * s;
        float modifier = bb_nomove < render_bounds.Min.x ? 1.0f : mod_frame.alpha;
        float bb_movepart = 20.0f * s * modifier;
        float grab_offset = std::clamp(bb_nomove + bb_movepart, render_bounds.Min.x, render_grab_bb.Max.x);
        draw_list->AddRectFilledMultiColor(render_bounds.Min, ImVec2(grab_offset, render_grab_bb.Max.y + 2.0f * s), gradient, 4.0f * s);

        float circle_radius = 8.0f * s;
        float circle_nomove = (render_grab_bb.Min.x + ((render_grab_bb.Max.x - render_grab_bb.Min.x) / 2.0f)) - 20.0f * s;
        float circle_offset = std::clamp(circle_nomove + bb_movepart, render_bounds.Min.x, render_grab_bb.Max.x);
        ImVec2 circle_pos = ImVec2(circle_offset, render_grab_bb.Min.y + 1.0f * s);
        draw_list->AddCircleFilled(circle_pos, circle_radius, c_color(75, 75, 75, (int)(255 * alpha)).as_imcolor());
        draw_list->AddCircleFilled(circle_pos, circle_radius - 3.0f * s, c_color(255, 255, 255, (int)(255 * alpha)).as_imcolor());
    }

    static std::map<uint32_t, int> input_values;
    static uint32_t input_focus = 0;
    ImGuiID edit_id = id ^ 0x9E3779B9u;
    ImRect value_bb(frame_bb.Max - ImVec2(64.0f * s, 48.0f * s), frame_bb.Max - ImVec2(8.0f * s, 28.0f * s));

    if (data_type == ImGuiDataType_S32 && input_values.find(edit_id) == input_values.end())
    {
        if (IsMouseHoveringRect(value_bb.Min, value_bb.Max) && IsMouseClicked(0))
        {
            input_values[edit_id] = *(int*)data;
            input_focus = edit_id;
        }
    }

    auto input_it = input_values.find(edit_id);

    if (data_type == ImGuiDataType_S32 && input_it != input_values.end())
    {
        ImVec2 backup_cursor = GetCursorScreenPos();
        SetCursorScreenPos(value_bb.Min + ImVec2(0, 1.0f * s));
        PushItemWidth(value_bb.GetWidth());
        PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.45f * alpha));
        PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.05f, 0.05f, 0.06f, 0.55f * alpha));
        PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.08f, 0.08f, 0.10f, 0.65f * alpha));
        c_color accent_sel = g_cfg.accent.to_color();
        PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(accent_sel.r() / 255.0f, accent_sel.g() / 255.0f, accent_sel.b() / 255.0f, 0.55f * alpha));

        if (input_focus == edit_id)
        {
            SetKeyboardFocusHere();
            input_focus = 0;
        }

        PushID((int)edit_id);
        bool done = ImGui::InputInt("##slider_value", &input_it->second, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll);
        bool active = IsItemActive();
        PopID();
        PopStyleColor(4);
        PopItemWidth();
        SetCursorScreenPos(backup_cursor);

        if (done || (IsItemDeactivated() && !active))
        {
            int clamped = std::clamp(input_it->second, *(int*)min, *(int*)max);

            if (clamped != *(int*)data)
            {
                *(int*)data = clamped;
                MarkItemEdited(id);
                value_changed = true;
            }

            input_values.erase(input_it);
            ClearActiveID();
        }
        else if (active && IsKeyPressed(ImGuiKey_Escape, false))
        {
            input_values.erase(input_it);
            ClearActiveID();
        }
    }
    else
    {
        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, data, format);
        ImVec2 size = CalcTextSize(value_buf);
        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
        RenderTextClipped(frame_bb.Max - ImVec2(size.x + 23.0f * s, 79.0f * s), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));
        PopStyleColor();
    }

    PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * alpha));

    if (label_size.x > 0.0f)
    {
        RenderText(ImVec2(frame_bb.Min.x + 12.0f * s, frame_bb.Min.y + 9.0f * s), label);
    }

    PopStyleColor();
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);

    return value_changed;
}

bool Menu::SliderInt(const char* label, int* value, int min, int max, const char* format)
{
    return SliderScalar(label, ImGuiDataType_S32, value, &min, &max, format);
}

bool Menu::SliderFloat(const char* label, float* value, float min, float max, const char* format)
{
    return SliderScalar(label, ImGuiDataType_Float, value, &min, &max, format);
}
