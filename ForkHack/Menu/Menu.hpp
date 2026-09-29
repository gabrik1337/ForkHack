#pragma once

#include <array>
#include <d3d9.h>
#include <map>
#include <string>
#include <vector>
#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "Utils/xorstr.h"

#include "Core/Color.hpp"
#include "Core/Config.hpp"

// todo: make a УКРАIНСКИ ЯЗIК ПО ПРИКОЛУ
// и желательно ЧЭЧЭНСКИ БОЛЯ 
inline const char* tr(const char* ru, const char* en)
{
    return g_cfg.language == 1 ? en : ru; // 0 = ru, 1 = english
}

struct tab_animation_t
{
    float hovered_alpha = 0.0f;
    float alpha = 0.0f;
    bool hovered = false;
    bool selected = false;
};

struct item_animation_t
{
    bool active = false;
    float hovered_alpha = 0.0f;
    float alpha = 0.0f;

    void reset()
    {
        active = false;
        hovered_alpha = 0.0f;
        alpha = 0.0f;
    }
};

constexpr auto default_picker = ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview |
    ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayRGB;

constexpr auto no_alpha_picker = ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview |
    ImGuiColorEditFlags_DisplayRGB;

enum anim_flags_t
{
    AnimSkipEnable = (1 << 0),
    AnimSkipDisable = (1 << 1),
    AnimLerp = (1 << 2)
};

class Menu
{
    int tab_selector = 0;
    ImDrawList* draw_list = nullptr;
    float alpha = 0.0f;
    float widget_alpha_mul = 1.0f;
    ImVec2 window_pos{};
    std::array<tab_animation_t, 5> tab_info{};
    std::map<std::string, std::array<tab_animation_t, 6>> subtab_info{};
    std::map<uint32_t, item_animation_t> item_animations{};
    std::map<std::string, std::string> combo_items{};
    float tab_alpha = 0.0f;
    std::array<float, 5> subtab_alpha{};
    int prev_tab = 0;
    std::array<int, 5> prev_sub{};
    LPDIRECT3DTEXTURE9 logo_texture = nullptr;
    LPDIRECT3DTEXTURE9 kb_texture = nullptr;
    std::array<LPDIRECT3DTEXTURE9, 5> icon_textures{};
    uint32_t Hash_Label(const char* text);
    static bool Vector_Getter(void* vec, int idx, const char** out_text);
    static bool Items_Array_Getter(void* data, int idx, const char** out_text);
    static float Calc_Max_Popup_Height(int items_count);
    static void Render_Vertical_Arrows(ImDrawList* list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha);

public:
    bool opened = true;

    bool GetState();
    bool ToggleState();
    void SetDrawList(ImDrawList* list);
    void SetWindowPos(const ImVec2& pos);
    ImVec2 GetWindowPos();
    void CreateAnimation(float& mod, bool cond, float speed, unsigned int flags);
    void UpdateAlpha();
    float GetAlpha();
    float GetScale();
    void ApplyScale();
    void InitTextures();
    void WindowBegin();
    void WindowEnd();
    void DrawBackground();
    void DrawTabs();
    void DrawSubTabs(int& selector, const std::vector<std::string>& tabs);
    void UpdateSubFade(int tab, int& sub);
    void GroupBegin(const char* label);
    void GroupEnd();
    void DrawContent();
    void DrawRage();
    void DrawLegit();
    void DrawVisuals();
    void DrawMisc();
    void DrawProfile();
    void Draw();
    void DrawBinds();
    bool Checkbox(const char* label, bool* value);
    bool BindableCheckbox(const char* id, const char* label, bool* v, keybind_t* bind = nullptr);
    bool SliderScalar(const char* label, ImGuiDataType data_type, void* data, const void* min, const void* max, const char* format = NULL, float power = 1.0f);
    bool SliderInt(const char* label, int* value, int min, int max, const char* format = "%d");
    bool SliderFloat(const char* label, float* value, float min, float max, const char* format = "%.1f");
    bool Selectable(const char* label, bool selected, float alpha_pass = 1.0f, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
    bool BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags, int item_cnt = 0);
    bool ComboWrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items);
    bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
    bool Selectable2(const char* label, bool* selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0), float alpha_pass = 1.0f);
    bool SelectableFlags(const char* label, unsigned int* flags, unsigned int flags_value, float alpha_pass = 1.0f);
    void MultiCombo(const char* label, unsigned int& var, std::vector<std::string> elements);
    bool Button(const char* label);
    bool ColorPickerWrapper(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
    bool ColorButtonWrapper(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0);
    bool ColorPicker(const char* label, c_float_color& value, ImGuiColorEditFlags flags = default_picker);
    bool Keybind(const char* label, keybind_t* bind);
    bool Listbox(const char* label, int* current, const char* const items[], int count, int height_in_items = 6);
    bool ListboxSelectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg, int iter);
    bool ListBoxHeader(const char* label, const ImVec2& size_arg);
    bool ListBoxHeaderStart(const char* label, int items_count, int height_in_items);
    void ListBoxFooter();
    bool ListBoxWrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items, const std::string& compare_text);
    bool Listbox(const char* label, int* current_item, std::vector<std::string>& values, int height_in_items, const std::string& compare_text);
    bool Textbox(const char* label, char* buf, int buf_size);
    bool InputInt(const char* label, int* value);
    bool InputTextWrapper(const char* label, const char* hint, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* callback_user_data);
};

extern Menu* menu;
