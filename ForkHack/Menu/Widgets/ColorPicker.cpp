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

static void To_Clipboard(const char* text)
{
    if (!text)
    {
        return;
    }

    if (OpenClipboard(0))
    {
        EmptyClipboard();

        char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));

        if (clip_data)
        {
            lstrcpy(clip_data, text);

            if (!SetClipboardData(CF_TEXT, (HANDLE)(clip_data)))
            {
                GlobalFree(clip_data);
            }
        }

        LCID* lcid = (LCID*)(GlobalAlloc(GMEM_FIXED, sizeof(LCID)));

        if (lcid)
        {
            *lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);

            if (!SetClipboardData(CF_LOCALE, (HANDLE)(lcid)))
            {
                GlobalFree(lcid);
            }
        }

        CloseClipboard();
    }
}

static std::string From_Clipboard()
{
    std::string fromClipboard = "";

    if (OpenClipboard(0))
    {
        HANDLE hData = GetClipboardData(CF_TEXT);

        if (hData)
        {
            char* chBuffer = (char*)GlobalLock(hData);

            if (chBuffer)
            {
                fromClipboard = chBuffer;
                GlobalUnlock(hData);
            }
        }

        CloseClipboard();
    }

    return fromClipboard;
}

static void Render_Checkerboard(ImVec2 p_min, ImVec2 p_max, c_color clr, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags, float alpha_mod = 1.0f)
{
    ImGuiWindow* window = GetCurrentWindow();
    c_color temp_col_bg1 = c_color(204, 204, 204).multiply(clr, clr.a() / 255.0f).new_alpha((int)(255 * alpha_mod));
    c_color temp_col_bg2 = c_color(128, 128, 128).multiply(clr, clr.a() / 255.0f).new_alpha((int)(255 * alpha_mod));
    ImU32 col_bg1 = GetColorU32(ImVec4(temp_col_bg1.r() / 255.0f, temp_col_bg1.g() / 255.0f, temp_col_bg1.b() / 255.0f, temp_col_bg1.a() / 255.0f));
    ImU32 col_bg2 = GetColorU32(ImVec4(temp_col_bg2.r() / 255.0f, temp_col_bg2.g() / 255.0f, temp_col_bg2.b() / 255.0f, temp_col_bg2.a() / 255.0f));
    window->DrawList->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

    int yi = 0;

    for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
    {
        float y1 = ImClamp(y, p_min.y, p_max.y);
        float y2 = ImMin(y + grid_step, p_max.y);

        if (y2 <= y1)
        {
            continue;
        }

        for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
        {
            float x1 = ImClamp(x, p_min.x, p_max.x);
            float x2 = ImMin(x + grid_step, p_max.x);

            if (x2 <= x1)
            {
                continue;
            }

            int rounding_corners_flags_cell = 0;

            if (y1 <= p_min.y)
            {
                if (x1 <= p_min.x)
                {
                    rounding_corners_flags_cell |= ImDrawCornerFlags_TopLeft;
                }

                if (x2 >= p_max.x)
                {
                    rounding_corners_flags_cell |= ImDrawCornerFlags_TopRight;
                }
            }

            if (y2 >= p_max.y)
            {
                if (x1 <= p_min.x)
                {
                    rounding_corners_flags_cell |= ImDrawCornerFlags_BotLeft;
                }

                if (x2 >= p_max.x)
                {
                    rounding_corners_flags_cell |= ImDrawCornerFlags_BotRight;
                }
            }

            rounding_corners_flags_cell &= rounding_corners_flags;
            window->DrawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
        }
    }
}

bool Menu::ColorPickerWrapper(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
    float window_alpha = GetAlpha();
    item_animation_t& picker_mod = item_animations[Hash_Label(label)];
    float picker_alpha = picker_mod.alpha;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    ImDrawList* picker_draw_list = window->DrawList;
    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    const float s = GetScale();
    const float width = 152.0f * s;
    g.NextItemData.ClearFlags();

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        flags |= ImGuiColorEditFlags_NoSmallPreview;
    }

    if (!(flags & ImGuiColorEditFlags_NoOptions))
    {
        ColorPickerOptionsPopup(col, flags);
    }

    if (!(flags & ImGuiColorEditFlags__PickerMask))
    {
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__PickerMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__PickerMask;
    }

    if (!(flags & ImGuiColorEditFlags__InputMask))
    {
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__InputMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__InputMask;
    }

    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__PickerMask));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));

    if (!(flags & ImGuiColorEditFlags_NoOptions))
    {
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);
    }

    int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
    bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
    ImVec2 picker_pos = window->DC.CursorPos + ImVec2(4.0f * s, 1.0f * s);
    float square_sz = 12.0f * s;
    float bars_width = square_sz;
    float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x));
    float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
    float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
    float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);
    float backup_initial_col[4];
    std::memcpy(backup_initial_col, col, components * sizeof(float));
    float wheel_thickness = sv_picker_size * 0.08f;
    float wheel_r_outer = sv_picker_size * 0.50f;
    float wheel_r_inner = wheel_r_outer - wheel_thickness;
    ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);
    float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
    ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f);
    ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f);
    ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f);
    float H = col[0];
    float S = col[1];
    float V = col[2];
    float R = col[0];
    float G = col[1];
    float B = col[2];

    if (flags & ImGuiColorEditFlags_InputRGB)
    {
        ColorConvertRGBtoHSV(R, G, B, H, S, V);

        if (S == 0 && memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0)
        {
            H = g.ColorEditLastHue;
        }
    }
    else if (flags & ImGuiColorEditFlags_InputHSV)
    {
        ColorConvertHSVtoRGB(H, S, V, R, G, B);
    }

    bool value_changed = false;
    bool value_changed_h = false;
    bool value_changed_sv = false;
    PushID(label);
    BeginGroup();
    {
        PushItemFlag(ImGuiItemFlags_NoNav, true);

        if (flags & ImGuiColorEditFlags_PickerHueBar)
        {
            InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));

            if (IsItemActive())
            {
                S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1.0f * s));
                V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1.0f * s));
                value_changed = value_changed_sv = true;
            }

            if (!(flags & ImGuiColorEditFlags_NoOptions))
            {
                OpenPopupOnItemClick("context");
            }

            SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
            InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));

            if (IsItemActive())
            {
                H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1.0f * s));
                value_changed = value_changed_h = true;
            }
        }

        if (alpha_bar)
        {
            SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
            InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));

            if (IsItemActive())
            {
                col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1.0f * s));
                value_changed = true;
            }
        }

        PopItemFlag();

        if (!(flags & ImGuiColorEditFlags_NoSidePreview))
        {
            SameLine(0, style.ItemInnerSpacing.x);
            BeginGroup();
        }

        if (!(flags & ImGuiColorEditFlags_NoLabel))
        {
            const char* label_display_end = FindRenderedTextEnd(label);

            if (label != label_display_end)
            {
                if ((flags & ImGuiColorEditFlags_NoSidePreview))
                {
                    SameLine(0, style.ItemInnerSpacing.x);
                }

                TextEx(label, label_display_end);
            }
        }

        if (value_changed_h || value_changed_sv)
        {
            if (flags & ImGuiColorEditFlags_InputRGB)
            {
                ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H, S > 0.0f ? S : 10 * 1e-6f, V > 0.0f ? V : 1e-6f, col[0], col[1], col[2]);
                g.ColorEditLastHue = H;
                std::memcpy(g.ColorEditLastColor, col, sizeof(float) * 3);
            }
            else if (flags & ImGuiColorEditFlags_InputHSV)
            {
                col[0] = H;
                col[1] = S;
                col[2] = V;
            }
        }

        bool value_changed_fix_hue_wrap = false;

        if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
        {
            PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
            ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions |
                ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
            ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;

            if (flags & ImGuiColorEditFlags_DisplayRGB || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
            {
                if (ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_DisplayRGB))
                {
                    value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
                    value_changed = true;
                }
            }

            if (flags & ImGuiColorEditFlags_DisplayHSV || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
            {
                value_changed |= ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_DisplayHSV);
            }

            if (flags & ImGuiColorEditFlags_DisplayHex || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
            {
                value_changed |= ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_DisplayHex);
            }

            PopItemWidth();
        }

        if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
        {
            float new_H = 0.0f;
            float new_S = 0.0f;
            float new_V = 0.0f;
            ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);

            if (new_H <= 0 && H > 0)
            {
                if (new_V <= 0 && V != new_V)
                {
                    ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
                }
                else if (new_S <= 0)
                {
                    ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
                }
            }
        }

        if (value_changed)
        {
            if (flags & ImGuiColorEditFlags_InputRGB)
            {
                R = col[0];
                G = col[1];
                B = col[2];
                ColorConvertRGBtoHSV(R, G, B, H, S, V);

                if (S == 0 && memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0)
                {
                    H = g.ColorEditLastHue;
                }
            }
            else if (flags & ImGuiColorEditFlags_InputHSV)
            {
                H = col[0];
                S = col[1];
                V = col[2];
                ColorConvertHSVtoRGB(H, S, V, R, G, B);
            }
        }

        const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha * window_alpha * picker_alpha);
        const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
        const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
        const ImU32 col_hues[6 + 1] = { IM_COL32(255, 0, 0, style_alpha8), IM_COL32(255, 255, 0, style_alpha8), IM_COL32(0, 255, 0, style_alpha8), IM_COL32(0, 255, 255, style_alpha8), IM_COL32(0, 0, 255, style_alpha8),
            IM_COL32(255, 0, 255, style_alpha8), IM_COL32(255, 0, 0, style_alpha8) };
        ImVec4 hue_color_f(1, 1, 1, style.Alpha * window_alpha * picker_alpha);
        ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
        ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
        ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha * window_alpha * picker_alpha));
        ImVec2 sv_cursor_pos;

        if (flags & ImGuiColorEditFlags_PickerHueBar)
        {
            picker_draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), col_white, hue_color32, hue_color32, col_white);
            picker_draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black);
            RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0.0f);
            sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x + 2.0f * s, picker_pos.x + sv_picker_size - 2.0f * s);
            sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * sv_picker_size), picker_pos.y + 2.0f * s, picker_pos.y + sv_picker_size - 2.0f * s);

            for (int i = 0; i < 6; ++i)
            {
                picker_draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1]);
            }

            float bar0_line_y = IM_ROUND(picker_pos.y + H * sv_picker_size);
            RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
            Render_Vertical_Arrows(picker_draw_list, ImVec2(bar0_pos_x - 1.0f * s, bar0_line_y), ImVec2(bars_triangles_half_sz + 1.0f * s, bars_triangles_half_sz), bars_width + 2.0f * s, style.Alpha * window_alpha * picker_alpha);
        }

        float sv_cursor_rad = 3.0f * s;
        picker_draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, col_white, 15);
        picker_draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1.0f * s, col_black, 15);

        if (alpha_bar)
        {
            float bar_alpha = ImSaturate(col[3]);
            ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
            picker_draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha, user_col32_striped_of_alpha, user_col32_striped_of_alpha & ~IM_COL32_A_MASK, user_col32_striped_of_alpha & ~IM_COL32_A_MASK);
            float bar1_line_y = IM_ROUND(picker_pos.y + (1.0f - bar_alpha) * sv_picker_size);
            RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
            Render_Vertical_Arrows(picker_draw_list, ImVec2(bar1_pos_x - 1.0f * s, bar1_line_y), ImVec2(bars_triangles_half_sz + 1.0f * s, bars_triangles_half_sz), bars_width + 2.0f * s, style.Alpha * window_alpha * picker_alpha);
        }
    }
    EndGroup();

    BeginGroup();
    {
        ImVec2 pos = window->Pos;
        ImVec2 size = window->Size;
        ImDrawListFlags old_flags = picker_draw_list->Flags;
        picker_draw_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
        ImVec2 button_size = ImVec2(76.0f * s, 24.0f * s);
        BeginGroup();
        {
            item_animation_t& copy_mod = item_animations[Hash_Label(label) + Hash_Label("Copy")];
            ImGuiID copy_id = window->GetID("##copybutton");
            ImVec2 copy_pos = window->DC.CursorPos;
            ImRect copy_bb(copy_pos, copy_pos + ImVec2(76.0f * s, 24.0f * s));
            ItemSize(ImVec2(76.0f * s, 24.0f * s), style.FramePadding.y);
            bool copy_hover = false;
            bool copy_button = false;

            if (ItemAdd(copy_bb, copy_id))
            {
                bool copy_held = false;
                copy_button = ButtonBehavior(copy_bb, copy_id, &copy_hover, &copy_held);
            }

            CreateAnimation(copy_mod.hovered_alpha, copy_hover, 1.0f, AnimLerp);

            float button_clr_alpha = (80 + 30 * copy_mod.hovered_alpha) * GetAlpha();
            c_color button_clr = c_color(35, 35, 35).increase((int)(20 * copy_mod.hovered_alpha)).new_alpha((int)button_clr_alpha);
            picker_draw_list->AddRectFilled(ImVec2(pos.x + 11.0f * s, (pos.y + size.y) - button_size.y - 6.0f * s), ImVec2(pos.x + 9.0f * s + button_size.x, (pos.y + size.y) - 6.0f * s), button_clr.as_imcolor(), 2.0f * s);
            picker_draw_list->AddText(ImVec2(pos.x + button_size.x / 2 - 6.0f * s, (pos.y + size.y) - button_size.y - 3.0f * s), c_color(255, 255, 255, (int)(255 * picker_alpha)).as_imcolor(), "Copy");

            if (copy_button)
            {
                std::string new_str = std::to_string(c_color(col[0] * 255.0f, col[1] * 255.0f, col[2] * 255.0f, col[3] * 255.0f).u32());
                To_Clipboard(new_str.c_str());
                CloseCurrentPopup();
            }
        }
        EndGroup();
        SameLine();
        BeginGroup();
        {
            item_animation_t& paste_mod = item_animations[Hash_Label(label) + Hash_Label("Paste")];
            ImGuiID paste_id = window->GetID("##pastebutton");
            ImVec2 paste_pos = window->DC.CursorPos;
            ImRect paste_bb(paste_pos, paste_pos + ImVec2(76.0f * s, 24.0f * s));
            ItemSize(ImVec2(76.0f * s, 24.0f * s), style.FramePadding.y);
            bool paste_hover = false;
            bool paste_button = false;

            if (ItemAdd(paste_bb, paste_id))
            {
                bool paste_held = false;
                paste_button = ButtonBehavior(paste_bb, paste_id, &paste_hover, &paste_held);
            }

            CreateAnimation(paste_mod.hovered_alpha, paste_hover, 1.0f, AnimLerp);

            float button_clr_alpha = (80 + 30 * paste_mod.hovered_alpha) * GetAlpha();
            c_color button_clr = c_color(35, 35, 35).increase((int)(20 * paste_mod.hovered_alpha)).new_alpha((int)button_clr_alpha);
            float base_pos = pos.x + button_size.x + 5.0f * s;
            picker_draw_list->AddRectFilled(ImVec2(base_pos + 11.0f * s, (pos.y + size.y) - button_size.y - 6.0f * s), ImVec2(base_pos + 9.0f * s + button_size.x, (pos.y + size.y) - 6.0f * s), button_clr.as_imcolor(), 2.0f * s);
            picker_draw_list->AddText(ImVec2(base_pos + button_size.x / 2 - 6.0f * s, (pos.y + size.y) - button_size.y - 3.0f * s), c_color(255, 255, 255, (int)(255 * picker_alpha)).as_imcolor(), "Paste");

            if (paste_button)
            {
                c_color clip_clr = c_color((uint32_t)std::atoll(From_Clipboard().c_str()));
                c_color temp_clr = c_color::hsb((float)clip_clr.hue(), (float)clip_clr.saturation(), (float)clip_clr.brightness()).new_alpha(clip_clr.a());
                col[0] = temp_clr.r() / 255.0f;
                col[1] = temp_clr.g() / 255.0f;
                col[2] = temp_clr.b() / 255.0f;
                col[3] = temp_clr.a() / 255.0f;
                CloseCurrentPopup();
            }
        }
        EndGroup();
        picker_draw_list->Flags = old_flags;
    }
    EndGroup();

    if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
    {
        value_changed = false;
    }

    if (value_changed)
    {
        MarkItemEdited(window->DC.LastItemId);
    }

    PopID();

    return value_changed;
}

bool Menu::ColorButtonWrapper(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * alpha));

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(desc_id);
    const ImVec2 picker_size = ImVec2(32.0f * s, 16.0f * s);
    const ImVec2 pos = window->DC.CursorPos;
    ImVec2 back_size = ImVec2(256.0f * s, 48.0f * s);
    const ImRect bb(pos + ImVec2(209.0f * s, 15.0f * s), pos + ImVec2(209.0f * s, 15.0f * s) + picker_size);
    const ImRect render_bb(pos, pos + back_size + ImVec2(0.0f, 4.0f * s));
    ItemSize(render_bb);

    if (!ItemAdd(render_bb, id))
    {
        return false;
    }

    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    bool hovered_frame = false;
    bool held_frame = false;
    ButtonBehavior(ImRect(pos, pos + back_size), id, &hovered_frame, &held_frame);
    item_animation_t& mod = item_animations[Hash_Label(desc_id) + Hash_Label("color")];
    CreateAnimation(mod.hovered_alpha, hovered_frame, 1.0f, AnimLerp);

    float back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);
    draw_list->AddRectFilled(pos, pos + back_size, back_clr.as_imcolor(), 4.0f * s);

    if (flags & ImGuiColorEditFlags_NoAlpha)
    {
        flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);
    }

    ImVec4 col_rgb = ImVec4(col.x, col.y, col.z, col.w * alpha);

    if (flags & ImGuiColorEditFlags_InputHSV)
    {
        ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);
    }

    ImVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, alpha);
    float grid_step = ImMin(picker_size.x, picker_size.y) / 2.99f;
    float rounding = 4.0f * s;
    ImRect bb_inner = bb;
    float off = -0.75f * s;
    bb_inner.Expand(off);

    if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f)
    {
        float mid_x = IM_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
        Render_Checkerboard(ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, c_color(col_rgb.x * 255, col_rgb.y * 255, col_rgb.z * 255, col_rgb.w * 255), grid_step, ImVec2(-grid_step + off, off), rounding, ImDrawCornerFlags_All, alpha);
        window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_rgb_without_alpha), rounding, ImDrawCornerFlags_All);
    }
    else
    {
        ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;

        if (col_source.w < 1.0f)
        {
            Render_Checkerboard(bb_inner.Min, bb_inner.Max, c_color(col_source.x * 255, col_source.y * 255, col_source.z * 255, col_source.w * 255), grid_step, ImVec2(off, off), rounding, ImDrawCornerFlags_All, alpha);
        }
        else
        {
            window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding, ImDrawCornerFlags_All);
        }
    }

    RenderText(ImVec2(pos.x + 12.0f * s, pos.y + 16.0f * s), desc_id);
    RenderNavHighlight(bb, id);
    PopStyleColor();

    return pressed;
}

bool Menu::ColorPicker(const char* label, c_float_color& value, ImGuiColorEditFlags flags)
{
    float window_alpha = GetAlpha();
    const float s = GetScale();

    ImGuiWindow* window = GetCurrentWindow();

    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float w_extra = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (g.FontSize * s + g.Style.FramePadding.y * 2.0f + style.ItemInnerSpacing.x);
    const float w_items_all = CalcItemWidth() - w_extra;
    const char* label_display_end = FindRenderedTextEnd(label);
    const bool use_alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = use_alpha ? 4 : 3;
    const ImGuiColorEditFlags flags_untouched = flags;
    BeginGroup();
    PushID(label);

    if (flags & ImGuiColorEditFlags_NoInputs)
    {
        flags = (flags & (~ImGuiColorEditFlags__InputMask)) | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoOptions;
    }

    if (!(flags & ImGuiColorEditFlags__InputMask))
    {
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputMask);
    }

    if (!(flags & ImGuiColorEditFlags__DataTypeMask))
    {
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
    }

    if (!(flags & ImGuiColorEditFlags__PickerMask))
    {
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
    }

    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask));

    float f[4] = { value[0], value[1], value[2], use_alpha ? value[3] / 255.0f : 1.0f };

    if (flags & ImGuiColorEditFlags_HSV)
    {
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    }

    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };
    bool value_changed = false;
    bool value_changed_as_float = false;
    ImDrawList* current_list = GetWindowDrawList();
    current_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
    bool picker_active = false;

    if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
    {
        if (!(flags & ImGuiColorEditFlags_NoInputs))
        {
            SameLine(0, style.ItemInnerSpacing.x);
        }

        const ImVec4 col_v4(value[0], value[1], value[2], use_alpha ? value[3] : 1.0f);
        std::string button_str = std::string(label) + "##color";
        bool enabled_picker = ColorButtonWrapper(button_str.c_str(), col_v4, flags);
        SetNextWindowBgAlpha(0.0f);

        if (enabled_picker)
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");
                SetNextWindowPos(window->DC.LastItemRect.GetBL() - ImVec2(-35.0f * s, 38.0f * s));
            }
        }

        if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
        {
            OpenPopup("context");
        }

        bool begin_popup = BeginPopupEx(GetCurrentWindow()->GetID("picker"), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        std::string picker_name = std::string(label) + "picker";
        item_animation_t& picker_mod = item_animations[Hash_Label(picker_name.c_str())];
        CreateAnimation(picker_mod.alpha, begin_popup, 0.3f, AnimSkipDisable | AnimLerp);

        ImVec2 base_pos = ImGui::GetWindowPos();
        GetWindowDrawList()->AddRectFilled(base_pos, base_pos + ImGui::GetWindowSize(), c_color(16, 16, 16, (int)(255 * window_alpha * picker_mod.alpha)).as_imcolor(), 10.0f * s);

        if (begin_popup)
        {
            picker_active = true;
            ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
            ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
            PushItemWidth(176.0f * s);
            value_changed |= ColorPickerWrapper(picker_name.c_str(), value.float_base(), picker_flags, &g.ColorPickerRef.x);
            PopItemWidth();
            EndPopup();
        }
    }

    current_list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

    if (!picker_active)
    {
        if (!value_changed_as_float)
        {
            for (int n = 0; n < 4; n++)
            {
                f[n] = i[n] / 255.0f;
            }
        }

        if (flags & ImGuiColorEditFlags_HSV)
        {
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        }

        if (value_changed)
        {
            value[0] = f[0];
            value[1] = f[1];
            value[2] = f[2];

            if (use_alpha)
            {
                value[3] = f[3];
            }
        }
    }

    PopID();
    EndGroup();

    return value_changed;
}
