#include "Menu/Menu.hpp"

#include "Utils/xorstr.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <shellapi.h>

#include "Assets/Assets.hpp"
#include "Gfx/Blur.hpp"
#include "Gfx/Fonts.hpp"
#include "Gfx/Texture.hpp"

#include "Game/Features.h"

Menu* menu = new Menu;

static bool s_dragging = false;
static ImVec2 s_drag_offset{};
static bool s_drag_pending = false;
static ImVec2 s_drag_pending_pos{};

uint32_t Menu::Hash_Label(const char* text)
{
    uint32_t hash = 2166136261u;

    while (*text)
    {
        hash ^= (uint8_t)(*text);
        hash *= 16777619u;
        text++;
    }

    return hash;
}

bool Menu::GetState()
{
    return g_cfg.menu_open;
}

bool Menu::ToggleState()
{
    g_cfg.menu_open = !g_cfg.menu_open;

    return g_cfg.menu_open;
}

void Menu::SetDrawList(ImDrawList* list)
{
    draw_list = list;
}

void Menu::SetWindowPos(const ImVec2& pos)
{
    window_pos = pos;
}

ImVec2 Menu::GetWindowPos()
{
    const float s = GetScale();
    return window_pos + ImVec2(45.0f * s, 15.0f * s);
}

void Menu::CreateAnimation(float& mod, bool cond, float speed, unsigned int flags)
{
    float time = (ImGui::GetIO().DeltaTime * 5.0f * 5.0f) * speed;

    if ((flags & AnimSkipEnable) && cond)
    {
        mod = 1.0f;
    }
    else if ((flags & AnimSkipDisable) && !cond)
    {
        mod = 0.0f;
    }

    if (flags & AnimLerp)
    {
        mod = mod + ((float)cond - mod) * time;
    }
    else
    {
        if (cond && mod <= 1.0f)
        {
            mod += time;
        }
        else if (!cond && mod >= 0.0f)
        {
            mod -= time;
        }
    }

    if (mod < 0.0f)
    {
        mod = 0.0f;
    }

    if (mod > 1.0f)
    {
        mod = 1.0f;
    }
}

void Menu::UpdateAlpha()
{
    CreateAnimation(alpha, g_cfg.menu_open, 0.3f, 0);
}

float Menu::GetAlpha()
{
    return alpha * widget_alpha_mul;
}

float Menu::GetScale()
{
    return g_cfg.ui_scale / 100.0f;
}

void Menu::ApplyScale()
{
    static int applied_scale = 0;

    if (applied_scale == g_cfg.ui_scale)
    {
        return;
    }

    applied_scale = g_cfg.ui_scale;
    ImGui::GetStyle() = ImGuiStyle();
    Fonts::InitStyle();
    ImGui::GetStyle().ScaleAllSizes(GetScale());
    ImGui::GetStyle().MouseCursorScale = GetScale();
}

bool Menu::Vector_Getter(void* vec, int idx, const char** out_text)
{
    std::vector<std::string>& vector = *static_cast<std::vector<std::string>*>(vec);

    if (idx < 0 || idx >= (int)vector.size())
    {
        return false;
    }

    *out_text = vector.at(idx).c_str();

    return true;
}

bool Menu::Items_Array_Getter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;

    if (out_text)
    {
        *out_text = items[idx];
    }

    return true;
}

float Menu::Calc_Max_Popup_Height(int items_count)
{
    ImGuiContext& g = *GImGui;

    if (items_count <= 0)
    {
        return FLT_MAX;
    }

    const float k = g.IO.FontGlobalScale > 0.0f ? g.IO.FontGlobalScale : 1.0f;
    return (g.FontSize + g.Style.ItemSpacing.y * k) * items_count - g.Style.ItemSpacing.y * k + (g.Style.WindowPadding.y * 2.0f * k);
}

void Menu::Render_Vertical_Arrows(ImDrawList* list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha)
{
    ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
    ImGui::RenderArrowPointingAt(list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32(0, 0, 0, alpha8));
    ImGui::RenderArrowPointingAt(list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32(255, 255, 255, alpha8));
    ImGui::RenderArrowPointingAt(list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32(0, 0, 0, alpha8));
    ImGui::RenderArrowPointingAt(list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32(255, 255, 255, alpha8));
}

void Menu::InitTextures()
{
    IDirect3DDevice9* device = Blur::GetDevice();

    if (!device)
    {
        return;
    }

    if (!logo_texture)
    {
        Texture::FromMemory(device, cheatLogo, sizeof(cheatLogo), &logo_texture);
    }

    if (!kb_texture)
    {
        Texture::FromMemory(device, keyboard_icon, sizeof(keyboard_icon), &kb_texture);
    }

    if (!icon_textures[0])
    {
        Texture::FromMemory(device, rage_icon, sizeof(rage_icon), &icon_textures[0]);
    }

    if (!icon_textures[1])
    {
        Texture::FromMemory(device, legit_icon, sizeof(legit_icon), &icon_textures[1]);
    }

    if (!icon_textures[2])
    {
        Texture::FromMemory(device, visuals_icon, sizeof(visuals_icon), &icon_textures[2]);
    }

    if (!icon_textures[3])
    {
        Texture::FromMemory(device, misc_icon, sizeof(misc_icon), &icon_textures[3]);
    }

    if (!icon_textures[4])
    {
        Texture::FromMemory(device, cfg_icon, sizeof(cfg_icon), &icon_textures[4]);
    }
}

void Menu::WindowBegin()
{
    const float s = GetScale();
    ImVec2 window_size = ImVec2(800.0f * s, 550.0f * s);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

    static int centered_scale = 0;
    static bool first_frame = true;

    if (first_frame || centered_scale != g_cfg.ui_scale)
    {
        first_frame = false;
        centered_scale = g_cfg.ui_scale;
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - window_size.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - window_size.y) * 0.5f));
    }
    if (s_drag_pending)
    {
        ImGui::SetNextWindowPos(s_drag_pending_pos);
        s_drag_pending = false;
    }

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushFont(g_fonts.main);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    static bool keep_open = true;
    ImGui::Begin("##base_window", &keep_open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    ImGui::PushItemWidth(256.0f * s);
    SetDrawList(ImGui::GetWindowDrawList());
    SetWindowPos(ImGui::GetWindowPos());

    ImDrawList* list = draw_list;
    list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

    ImVec2 wp = GetWindowPos();
    list->PushClipRect(wp, ImVec2(wp.x + 720.0f * s, wp.y + 520.0f * s));
    ImGui::PushClipRect(wp, ImVec2(wp.x + 720.0f * s, wp.y + 520.0f * s), false);
}

void Menu::WindowEnd()
{
    ImDrawList* list = draw_list;
    list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);
    list->PopClipRect();
    ImGui::PopClipRect();
    ImGui::PopItemWidth();
    ImGui::End(false);

    ImGui::PopStyleColor();
    ImGui::PopFont();
    draw_list = nullptr;
}

void Menu::DrawBackground()
{
    const float s = GetScale();
    ImDrawList* list = draw_list;
    float alpha = GetAlpha();
    float window_alpha = 255.0f * GetAlpha();
    ImVec2 window_pos = GetWindowPos();
    ImVec2 header_size = ImVec2(720.0f * s, 47.0f * s);
    Blur::Create(list, window_pos, ImVec2(window_pos.x + header_size.x, window_pos.y + header_size.y), c_color(255, 255, 255, window_alpha).as_imcolor(), 6.0f * s, ImDrawCornerFlags_Top);

    ImVec2 image_size = ImVec2(14.0f * s, 14.0f * s);
    ImVec2 image_pos_min = ImVec2((header_size.x / 2) - (image_size.x - 2.0f * s), (header_size.y / 2) - (image_size.y - 2.0f * s));
    ImVec2 image_pos_max = ImVec2((header_size.x / 2) + (image_size.x - 2.0f * s), (header_size.y / 2) + (image_size.y - 2.0f * s));
    c_color clr = g_cfg.accent.to_color();

    static const std::string letter_s =
#ifdef BETA_BUILD
        "ForkHack (beta)";
#else
        "ForkHack";
#endif
    const char* letter = letter_s.c_str();

    ImGui::PushFont(g_fonts.dmg);
    ImVec2 text_size = ImGui::CalcTextSize(letter);

    if (logo_texture)
    {
        list->AddImage((void*)logo_texture, window_pos + image_pos_min - ImVec2(text_size.x - 18.0f * s, 0.0f), window_pos + image_pos_max - ImVec2(text_size.x - 18.0f * s, 0.0f), ImVec2(0, 0), ImVec2(1, 1), clr.new_alpha(window_alpha).as_imcolor());
    }

    float base_x = window_pos.x + image_pos_min.x - text_size.x + 18.0f * s;
    list->AddText(ImVec2(base_x + 30.0f * s, window_pos.y + 13.0f * s), c_color(255, 255, 255, 150.0f * alpha).as_imcolor(), letter);
    ImGui::PopFont();

    Blur::Create(list, window_pos + ImVec2(0, 47.0f * s), ImVec2(window_pos.x + 720.0f * s, window_pos.y + 520.0f * s), ImColor(80, 80, 80, (int)(window_alpha)), 6.0f * s, ImDrawCornerFlags_Bot);
    list->AddLine(window_pos + ImVec2(0, 46.0f * s), window_pos + ImVec2(720.0f * s, 46.0f * s), c_color(255, 255, 255, 12.75f * alpha).as_imcolor());
    list->AddLine(window_pos + ImVec2(160.0f * s, 47.0f * s), window_pos + ImVec2(160.0f * s, 520.0f * s), c_color(255, 255, 255, 12.75f * alpha).as_imcolor(), 1.0f * s);
    list->AddRect(window_pos, ImVec2(window_pos.x + 720.0f * s, window_pos.y + 520.0f * s), c_color(100, 100, 100, 100.0f * alpha).as_imcolor(), 6.0f * s);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 header_max = ImVec2(window_pos.x + header_size.x, window_pos.y + header_size.y);

    if (!s_dragging && ImGui::IsMouseHoveringRect(window_pos, header_max) && ImGui::IsMouseClicked(0))
    {
        s_dragging = true;
        s_drag_offset = ImVec2(io.MousePos.x - window_pos.x, io.MousePos.y - window_pos.y);
    }

    s_drag_pending = false;

    if (s_dragging)
    {
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
        {
            s_drag_pending = true;
            s_drag_pending_pos = ImVec2(io.MousePos.x - s_drag_offset.x - 45.0f * s, io.MousePos.y - s_drag_offset.y - 15.0f * s);
        }
        else
        {
            s_dragging = false;
        }
    }
}

void Menu::DrawTabs()
{
    const float s = GetScale();
    ImDrawList* list = draw_list;
    ImVec2 prev = ImGui::GetCursorPos();
    ImVec2 child_pos = GetWindowPos() + ImVec2(0, 49.0f * s);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, GetAlpha()));

        const std::string names[5] = { tr("Рейдж", "Rage"), tr("Легит", "Legit"), tr("Визуалы", "Visuals"), tr("Разное", "Misc"), tr("Профиль", "Profile") };
    c_color accent = g_cfg.accent.to_color();

    for (int i = 0; i < 5; i++)
    {
        tab_animation_t& info = tab_info[i];
        ImGui::SetCursorPos(ImVec2(53.0f * s, (78.0f + 40.0f * i) * s));

        std::string id = std::string("##tab_") + std::to_string(i);
        bool clicked = ImGui::ButtonEx(id.c_str(), ImVec2(144.0f * s, 32.0f * s), 0, &info.hovered);

        if (clicked)
        {
            tab_selector = i;
        }

        CreateAnimation(info.hovered_alpha, info.hovered, 1.0f, AnimLerp);
        CreateAnimation(info.alpha, tab_selector == i, 0.8f, AnimSkipDisable | AnimLerp);

        ImVec2 tmin = child_pos + ImVec2(8.0f * s, (14.0f + 40.0f * i) * s);
        ImVec2 tmax = child_pos + ImVec2(152.0f * s, (46.0f + 40.0f * i) * s);
        float rgb = tab_selector == i ? 255 : 150 + 105 * info.hovered_alpha;
        c_color text_clr = c_color((int)rgb, (int)rgb, (int)rgb, (int)(rgb * GetAlpha()));

        if (tab_selector == i)
        {
            list->AddRectFilled(tmin, tmax - ImVec2(2.0f * s, 0), c_color(255, 255, 255, 10 * info.alpha * GetAlpha()).as_imcolor(), 4.0f * s, ImDrawCornerFlags_Left);
            list->PushClipRect(tmax - ImVec2(2.0f * s, 32.0f * s), tmax);
            list->AddRectFilled(tmax - ImVec2(4.0f * s, 32.0f * s), tmax, accent.new_alpha((int)(255 * info.alpha * GetAlpha())).as_imcolor(), 2.0f * s, ImDrawCornerFlags_Right);
            list->PopClipRect();
        }

        if (i == 4)
        {
            ImVec2 ip0 = tmin + ImVec2(10.0f * s, 9.0f * s);
            list->AddCircleFilled(ip0 + ImVec2(7.5f * s, 4.0f * s), 2.8f * s, text_clr.as_imcolor());
            list->AddRectFilled(ip0 + ImVec2(3.0f * s, 8.5f * s), ip0 + ImVec2(12.0f * s, 15.0f * s), text_clr.as_imcolor(), 3.0f * s);
        }
        else if (icon_textures[i])
        {
            list->AddImage((void*)icon_textures[i], tmin + ImVec2(10.0f * s, 9.0f * s), tmin + ImVec2(25.0f * s, 24.0f * s), ImVec2(0, 0), ImVec2(1, 1), text_clr.as_imcolor());
        }

        list->AddText(ImVec2(tmin.x + 32.0f * s, tmin.y + 8.0f * s), text_clr.as_imcolor(), names[i].c_str());
    }

    ImGui::PopStyleColor(4);
    ImGui::SetCursorPos(prev);
}

void Menu::DrawSubTabs(int& selector, const std::vector<std::string>& tabs)
{
    const float s = GetScale();
    ImGuiStyle& style = ImGui::GetStyle();
    float a = GetAlpha();
    float wa = 255.0f * a;
    ImVec2 child_pos = GetWindowPos() + ImVec2(178.0f * s, 62.0f * s);
    ImVec2 prev = ImGui::GetCursorPos();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, a));
    draw_list->AddRectFilled(child_pos, child_pos + ImVec2(528.0f * s, 58.0f * s), ImColor(217, 217, 217, (int)(20 * a)), 4.0f * s);

    c_color accent = g_cfg.accent.to_color();

    for (int i = 0; i < (int)tabs.size(); i++)
    {
        tab_animation_t& info = subtab_info[tabs[0]][i];
        ImVec2 cur = ImVec2((27.0f + 80.0f * i) * s, 14.0f * s);
        ImGui::SetCursorPos(cur);
        ImVec2 tsize = ImVec2(70.0f * s, 32.0f * s);

        std::string id = std::string("##sub_") + tabs[i];
        bool clicked = ImGui::ButtonEx(id.c_str(), tsize, 0, &info.hovered);

        if (clicked)
        {
            selector = i;
        }

        CreateAnimation(info.hovered_alpha, info.hovered, 1.0f, AnimLerp);
        CreateAnimation(info.alpha, selector == i, 0.8f, AnimSkipDisable | AnimLerp);

        ImVec2 tmin = child_pos + cur - ImVec2(8.0f * s, 0);
        ImVec2 tmax = tmin + tsize;
        ImRect bb = ImRect(tmin, tmax);

        if (selector == i)
        {
            draw_list->AddRectFilled(bb.Min, bb.Max, ImColor(217, 217, 217, (int)(20 * a * info.alpha)), 4.0f * s);
            draw_list->PushClipRect(ImVec2(bb.Min.x + 15.0f * s, bb.Max.y - 2.0f * s), ImVec2(bb.Max.x - 15.0f * s, bb.Max.y));
            draw_list->AddRectFilled(ImVec2(bb.Min.x + 15.0f * s, bb.Max.y - 2.0f * s), ImVec2(bb.Max.x - 15.0f * s, bb.Max.y + 4.0f * s), accent.new_alpha((int)(info.alpha * wa)).as_imcolor(), 2.0f * s, ImDrawCornerFlags_Top);
            draw_list->PopClipRect();
        }

        float rgb = selector == i ? 255 : 150 + 105 * info.hovered_alpha;
        c_color tc = c_color((int)rgb, (int)rgb, (int)rgb, (int)(rgb * a));
        ImVec2 ls = ImGui::CalcTextSize(tabs[i].c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(tc.r() / 255.0f, tc.g() / 255.0f, tc.b() / 255.0f, tc.a() / 255.0f));
        ImGui::RenderTextClipped(bb.Min, bb.Max, tabs[i].c_str(), NULL, &ls, style.ButtonTextAlign, &bb);
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor(4);
    ImGui::SetCursorPos(prev);
    ImGui::ItemSize(ImVec2(0, 62.0f * s));
    ImGui::PushClipRect(child_pos + ImVec2(0.0f, 62.0f * s), child_pos + ImVec2(540.0f * s, 457.0f * s), false);
    draw_list->PushClipRect(child_pos + ImVec2(0.0f, 62.0f * s), child_pos + ImVec2(540.0f * s, 457.0f * s));
}

void Menu::UpdateSubFade(int tab, int& sub)
{
    if (sub != prev_sub[tab])
    {
        for (auto& a : item_animations)
        {
            a.second.reset();
        }

        subtab_alpha[tab] = 0.0f;
        prev_sub[tab] = sub;
    }

    CreateAnimation(subtab_alpha[tab], g_cfg.menu_open && sub == prev_sub[tab], 0.2f, AnimSkipDisable | AnimLerp);
    alpha = subtab_alpha[tab] * tab_alpha;
}

void Menu::GroupBegin(const char* label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, GetAlpha()));
    ImGui::Text(label);
    ImGui::PopStyleColor();
    ImGui::ItemSize(ImVec2(0, 1.0f * GetScale()));
    ImGui::BeginGroup();
}

void Menu::GroupEnd()
{
    ImGui::EndGroup();
    ImGui::ItemSize(ImVec2(0, 9.0f * GetScale()));
}

void Menu::DrawContent()
{
    ImVec2 window_pos = GetWindowPos();
    ImVec2 prev_pos = ImGui::GetCursorPos();
    float old_alpha = GetAlpha();

    if (tab_selector != prev_tab)
    {
        for (auto& a : item_animations)
        {
            a.second.reset();
        }

        tab_alpha = 0.0f;
        prev_tab = tab_selector;
    }

    CreateAnimation(tab_alpha, g_cfg.menu_open && tab_selector == prev_tab, 0.2f, AnimSkipDisable | AnimLerp);
    alpha = tab_alpha;

    const float s = GetScale();
    ImGui::SetCursorPos(ImVec2(215.0f * s, 77.0f * s));
    ImRect window_bb = ImRect(window_pos + ImVec2(160.0f * s, 47.0f * s), window_pos + ImVec2(725.0f * s, 520.0f * s));
    ImGui::PushClipRect(window_bb.Min, window_bb.Max, false);
    ImGui::BeginChild("##tab_child", ImVec2(), true);

    if (tab_selector == 0)
    {
        DrawRage();
    }
    else if (tab_selector == 1)
    {
        DrawLegit();
    }

    else if (tab_selector == 2)
    {
        DrawVisuals();
    }

    else if (tab_selector == 3)
    {
        DrawMisc();
    }

    else if (tab_selector == 4)
    {
        DrawProfile();
    }

    ImGui::EndChild(false);
    alpha = old_alpha;
    ImGui::PopClipRect();
    ImGui::SetCursorPos(prev_pos);
}

void Menu::Draw()
{
    InitTextures();
    UpdateAlpha();

    if (!g_cfg.menu_open && alpha <= 0.01f)
    {
        return;
    }

    ApplyScale();
    ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayRGB);
    WindowBegin();
    DrawBackground();
    DrawTabs();
    DrawContent();
    draw_list->PopClipRect();
    ImGui::PopClipRect();
    WindowEnd();
}




