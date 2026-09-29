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

bool Menu::Checkbox(const char* label, bool* value)
{
    float alpha = GetAlpha();
    const float s = GetScale();
    PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * alpha));

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
    const ImRect total_bb(pos, pos + ImVec2(256.0f * s, 36.0f * s));
    ItemSize(total_bb, style.FramePadding.y);

    if (!ItemAdd(total_bb, id))
    {
        PopStyleColor();

        return false;
    }

    item_animation_t& mod = item_animations[Hash_Label(label)];
    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    if (pressed)
    {
        *value = !(*value);
        MarkItemEdited(id);
    }

    CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);
    CreateAnimation(mod.alpha, *value, 0.4f, AnimLerp);

    ImVec2 back_size = ImVec2(256.0f * s, 32.0f * s);
    float back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
    c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);
    draw_list->AddRectFilled(pos, pos + back_size, back_clr.as_imcolor(), 4.0f * s);

    ImVec2 body_size = ImVec2(28.0f * s, 14.0f * s);
    ImVec2 body_min = pos + ImVec2(220.0f * s, 9.0f * s);
    ImVec2 body_max = body_min + body_size;
    draw_list->AddRectFilled(body_min, body_max, c_color(0, 0, 0, (int)(80 * alpha)).as_imcolor(), 8.0f * s);

    c_color accent = g_cfg.accent.to_color();
    ImVec2 circle_offset = ImVec2(14.0f * s * mod.alpha, 0);
    ImVec2 circle_pos = body_min + ImVec2(body_size.y * 0.5f, body_size.y * 0.5f) + circle_offset;
    c_color circle_clr = c_color().multiply(accent, mod.alpha).new_alpha((int)(255 * alpha));
    draw_list->AddCircleFilled(circle_pos, 4.0f * s, circle_clr.as_imcolor());

    if (label_size.x > 0.0f)
    {
        RenderText(ImVec2(pos.x + 12.0f * s, pos.y + 7.0f * s), label);
    }

    PopStyleColor();
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*value ? ImGuiItemStatusFlags_Checked : 0));

    return pressed;
}

bool Menu::BindableCheckbox(const char* id, const char* label, bool* v, keybind_t* bind)
{
    bool pressed = Checkbox(label, v);

    if (pressed && bind != nullptr)
    {
        bind->manual = *v;
    }

    if (bind != nullptr)
    {
        if (IsItemHovered() && IsMouseClicked(1))
        {
            OpenPopup(id);
        }

        const float pop_alpha = GetAlpha();
        PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (BeginPopup(id))
        {
            const float s = GetScale();
            ImDrawList* plist = GetWindowDrawList();
            ImVec2 card_min = ImGui::GetWindowPos();
            ImVec2 card_max = card_min + ImVec2(216.0f * s, 110.0f * s);
            plist->AddRectFilled(card_min, card_max, c_color(105, 105, 110, (int)(255.0f * pop_alpha)).as_imcolor(), 6.0f * s);
            Blur::Create(plist, card_min, card_max, ImColor(80, 80, 80, (int)(255.0f * pop_alpha)), 6.0f * s);
            ImGuiWindow* window = GetCurrentWindow();

            PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, pop_alpha));
            ImGui::Text(label);
            PopStyleColor();
            ItemSize(ImVec2(0, 1.0f * s));

            if (!window->SkipItems)
            {
                ImGuiContext& g = *GImGui;
                const ImGuiStyle& style = g.Style;
                const std::string idStr = id;
                bool changed = false;

                {
                    ImGuiID keyId = window->GetID((idStr + "_key").c_str());
                    ImVec2 pos = window->DC.CursorPos;
                    ImRect total_bb(pos, pos + ImVec2(200.0f * s, 30.0f * s));
                    ItemSize(total_bb, style.FramePadding.y);

                    if (ItemAdd(total_bb, keyId))
                    {
                        uint32_t h = Hash_Label((idStr + "_key").c_str());
                        item_animation_t& mod = item_animations[h];
                        ImRect key_bb(pos + ImVec2(128.0f * s, 3.0f * s), pos + ImVec2(196.0f * s, 27.0f * s));
                        bool hovered = false;
                        bool held = false;
                        bool pressed = ButtonBehavior(key_bb, keyId, &hovered, &held);
                        CreateAnimation(mod.hovered_alpha, hovered, 1.0f, AnimLerp);

                        static std::map<uint32_t, int> capture;
                        int& state = capture[h];

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

                        float back_alpha = (20 + (30 * mod.hovered_alpha)) * pop_alpha;
                        c_color back_clr = c_color(217, 217, 217).increase((int)(38 * mod.hovered_alpha)).new_alpha((int)back_alpha);
                        plist->AddRectFilled(pos, pos + ImVec2(200.0f * s, 30.0f * s), back_clr.as_imcolor(), 4.0f * s);

                        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * pop_alpha));
                        RenderText(ImVec2(pos.x + 10.0f * s, pos.y + 7.0f * s), tr("Бинд", "Bind"));

                        static const std::string dots_s = "...";
                        const char* shown = state > 0 ? dots_s.c_str() : KeyName(bind->key);
                        ImVec2 shown_size = CalcTextSize(shown);
                        plist->AddRectFilled(key_bb.Min, key_bb.Max, c_color(0, 0, 0, (int)(80 * pop_alpha)).as_imcolor(), 3.0f * s);
                        RenderTextClipped(key_bb.Min, key_bb.Max, shown, NULL, &shown_size, ImVec2(0.5f, 0.5f));
                        PopStyleColor();
                    }
                }

                {
                    ImVec2 pos = window->DC.CursorPos;
                    ImRect total_bb(pos, pos + ImVec2(200.0f * s, 30.0f * s));
                    ItemSize(total_bb, style.FramePadding.y);

                    if (ItemAdd(total_bb, window->GetID((idStr + "_mode").c_str())))
                    {
                        const std::string t0 = tr("Нажимать", "Toggle");
                        const std::string t1 = tr("Зажимать", "Hold");
                        ImRect left_bb(pos + ImVec2(0.0f, 3.0f * s), pos + ImVec2(98.0f * s, 27.0f * s));
                        ImRect right_bb(pos + ImVec2(102.0f * s, 3.0f * s), pos + ImVec2(200.0f * s, 27.0f * s));

                        uint32_t h = Hash_Label((idStr + "_mode").c_str());
                        item_animation_t& mod = item_animations[h];
                        bool lh = false;
                        bool lheld = false;
                        bool rh = false;
                        bool rheld = false;
                        bool lp = ButtonBehavior(left_bb, window->GetID((idStr + "_ml").c_str()), &lh, &lheld);
                        bool rp = ButtonBehavior(right_bb, window->GetID((idStr + "_mr").c_str()), &rh, &rheld);
                        CreateAnimation(mod.hovered_alpha, lh || rh, 1.0f, AnimLerp);

                        c_color accent = g_cfg.accent.to_color();
                        c_color off_clr = c_color(217, 217, 217).new_alpha((int)(20 * pop_alpha));
                        c_color on_clr = accent.new_alpha((int)(200 * pop_alpha));
                        plist->AddRectFilled(left_bb.Min, left_bb.Max, (bind->mode == 0 ? on_clr : off_clr).as_imcolor(), 3.0f * s);
                        plist->AddRectFilled(right_bb.Min, right_bb.Max, (bind->mode == 1 ? on_clr : off_clr).as_imcolor(), 3.0f * s);

                        ImVec2 t0_size = CalcTextSize(t0.c_str());
                        ImVec2 t1_size = CalcTextSize(t1.c_str());
                        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, (bind->mode == 0 ? 1.0f : 0.5f) * pop_alpha));
                        RenderTextClipped(left_bb.Min, left_bb.Max, t0.c_str(), NULL, &t0_size, ImVec2(0.5f, 0.5f));
                        PopStyleColor();
                        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, (bind->mode == 1 ? 1.0f : 0.5f) * pop_alpha));
                        RenderTextClipped(right_bb.Min, right_bb.Max, t1.c_str(), NULL, &t1_size, ImVec2(0.5f, 0.5f));
                        PopStyleColor();

                        if (lp && bind->mode != 0)
                        {
                            bind->mode = 0;
                            changed = true;
                        }

                        if (rp && bind->mode != 1)
                        {
                            bind->mode = 1;
                            changed = true;
                        }
                    }
                }

                if (changed)
                {
                    SaveGeneralConfig();
                }
            }

            plist->AddRect(card_min, card_max, c_color(100, 100, 100, (int)(100.0f * pop_alpha)).as_imcolor(), 6.0f * s);
            EndPopup();
        }

        PopStyleColor(2);
    }

    return pressed;
}
