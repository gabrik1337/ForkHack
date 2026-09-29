#include "Gfx/Fonts.hpp"

#include "Assets/Assets.hpp"

Fonts g_fonts;

void Fonts::Init()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 3;
    main = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 14.0f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    dmg = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 18.0f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
}

void Fonts::InitStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.85f, 0.85f, 0.85f, 0.85f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.70f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.14f, 0.16f, 0.19f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.24f, 0.28f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.70f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.07f, 0.70f);
    style.Alpha = 1.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.FramePadding = ImVec2(1, 1);
    style.ScrollbarSize = 10.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabMinSize = 5.0f;
}
