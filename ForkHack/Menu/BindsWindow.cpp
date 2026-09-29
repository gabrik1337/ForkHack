#include "Menu/Menu.hpp"

#include "Game/Features.h"

#include "Gfx/Blur.hpp"
#include "Gfx/Fonts.hpp"

#include <cmath>

static std::string Bind_Display_Name(int i)
{
    switch (i)
    {
    case 0: return tr("Годмод", "Godmode");
    case 1: return tr("Быстрый бег", "Fast run");
    case 2: return tr("Валлхак", "Wallhack");
    case 3: return tr("Спидхак", "Speedhack");
    case 4: return tr("Аирбрейк", "Airbreak");
    case 5: return tr("Поменять скин", "Change skin");
    case 6: return tr("Найтмод", "Nightmode");
    case 7: return tr("Своё время", "Custom time");
    case 8: return tr("Свет и вода", "Light & water");
    case 9: return tr("Небо", "Sky");
    case 10: return tr("Рапид", "Rapid fire");
    case 11: return tr("Скорость игры", "Game speed");
    case 12: return tr("Быстрая ротация", "Fast Rotation");
    case 13: return tr("Не падать с байка", "No bike fall");
    case 14: return tr("Машины по воде", "Drive on water");
    case 15: return tr("Летающие машины", "Flying cars");
    case 16: return tr("Анти-колизия", "No collision");
    case 17: return tr("Чистка мира", "World Removals");
    case 18: return tr("Бесконечные патроны", "Infinite ammo");
    case 19: return tr("Свой цвет", "Custom color");
    case 20: return tr("Фулбрайт", "Fullbright");
    case 21: return tr("Аспект", "Aspect");
    case 22: return tr("Автозавод", "Auto engine");
    case 23: return tr("Автооткрытие", "Auto unlock");
    case 24: return tr("Быстрый прицел", "Fast crosshair");
    case 25: return tr("Без отдачи", "No recoil");
    case 26: return tr("Без разброса", "No spread");
    case 27: return tr("Фов", "Fov");
    case 28: return tr("Рванка", "Rvanka");
    case 29: return tr("Заморозить время", "Freeze time");
    case 30: return tr("Туман", "Fog");
    case 31: return tr("Солнце", "Sun");
    case 32: return tr("Триггербот", "Triggerbot");
    case 33: return tr("Без колизии камеры", "No camera collision");
    case 34: return tr("Камхак", "Camhack");
    case 35: return tr("Без урона от падения", "No fall damage");
    default: return "";
    }
}

void Menu::DrawBinds()
{
    if (!g_cfg.showbinds)
    {
        return;
    }

    struct bind_ref_t
    {
        keybind_t* bind;
        bool* value;
        bool vehicle;
    };

    static const bind_ref_t kBinds[] = {
        { &g_cfg.godmode_bind, &g_cfg.godmode, false },
        { &g_cfg.fastbeg_bind, &g_cfg.fastbeg, false },
        { &g_cfg.wh_bind, &g_cfg.wh, false },
        { &g_cfg.speedhack_bind, &g_cfg.speedhack, true },
        { &g_cfg.airbreake_bind, &g_cfg.airbreake, false },
        { &g_cfg.changemodel_bind, &g_cfg.changemodel, false },
        { &g_cfg.nightmode_bind, &g_cfg.nightmode, false },
        { &g_cfg.customtime_bind, &g_cfg.customtime, false },
        { &g_cfg.customcolor_bind, &g_cfg.customcolor, false },
        { &g_cfg.skychange_bind, &g_cfg.skychange, false },
        { &g_cfg.rapidfire_bind, &g_cfg.rapidfire, false },
        { &g_cfg.gamespeed_bind, &g_cfg.gamespeed, false },
        { &g_cfg.fastrot_bind, &g_cfg.fastrot, false },
        { &g_cfg.nobikefall_bind, &g_cfg.nobikefall, true },
        { &g_cfg.waterdrive_bind, &g_cfg.waterdrive, true },
        { &g_cfg.carfly_bind, &g_cfg.carfly, true },
        { &g_cfg.nocol_bind, &g_cfg.nocol, false },
        { &g_cfg.removals_bind, &g_cfg.removals, false },
        { &g_cfg.infammo_bind, &g_cfg.infammo, false },
        { &g_cfg.carcolor_bind, &g_cfg.carcolor, true },
        { &g_cfg.fullbright_bind, &g_cfg.fullbright, false },
        { &g_cfg.aspect_bind, &g_cfg.aspect, false },
        { &g_cfg.autoengine_bind, &g_cfg.autoengine, true },
        { &g_cfg.autounlock_bind, &g_cfg.autounlock, false },
        { &g_cfg.fastcross_bind, &g_cfg.fastcross, false },
        { &g_cfg.norecoil_bind, &g_cfg.norecoil, false },
        { &g_cfg.nospread_bind, &g_cfg.nospread, false },
        { &g_cfg.fov_bind, &g_cfg.fov, false },
        { &g_cfg.ram_bind, &g_cfg.ram, true },
        { &g_cfg.freezetime_bind, &g_cfg.freezetime, false },
        { &g_cfg.fogchange_bind, &g_cfg.fogchange, false },
        { &g_cfg.suncolor_bind, &g_cfg.suncolor, false },
        { &g_cfg.trigger_bind, &g_cfg.trigger, false },
        { &g_cfg.nocamcol_bind, &g_cfg.nocamcol, false },
        { &g_cfg.camhack_bind, &g_cfg.camhack, false },
        { &g_cfg.nofall_bind, &g_cfg.nofall, false },
    };

    static const int kBindCount = sizeof(kBinds) / sizeof(kBinds[0]);

    CPed* pBindLocal = FindPlayerPed();
    bool inVehicle = pBindLocal && pBindLocal->m_pVehicle != nullptr;

    int vis[64];
    int nvis = 0;

    for (int i = 0; i < kBindCount && nvis < 64; i++)
    {
        if (kBinds[i].bind->key < 0)
        {
            continue;
        }

        if (kBinds[i].vehicle && !inVehicle)
        {
            continue;
        }

        bool on = kBinds[i].bind->mode == 1 ? ((GetAsyncKeyState(kBinds[i].bind->key) & 0x8000) != 0) : *kBinds[i].value;

        if (on)
        {
            vis[nvis++] = i;
        }
    }

    static float bind_alpha = 0.0f;
    CreateAnimation(bind_alpha, g_cfg.menu_open || nvis > 0, 1.0f, AnimLerp);

    if (bind_alpha <= 0.0f)
    {
        return;
    }

    const float s = GetScale();
    const float row_h = 25.0f * s;
    const float head_h = 32.0f * s;
    const float rows_top = 40.0f * s;
    const float pad_l = 16.0f * s;
    const float pad_r = 8.0f * s;
    const float gap = 10.0f * s;

    ImGui::PushFont(g_fonts.main);

    float max_name = 0.0f, max_mode = 0.0f;

    for (int vi = 0; vi < nvis; vi++)
    {
        int i = vis[vi];
        max_name = ImMax(max_name, ImGui::CalcTextSize(Bind_Display_Name(i).c_str()).x);
        std::string m = kBinds[i].bind->mode == 1 ? tr("[ холд ]", "[ hold ]") : tr("[ тогл ]", "[ toggled ]");
        max_mode = ImMax(max_mode, ImGui::CalcTextSize(m.c_str()).x);
    }

    const float content_w = ImMax(pad_l + max_name + gap + max_mode + pad_r, 150.0f * s);
    const float win_w = content_w + 8.0f * s;

    static float win_h_anim = 40.0f;
    float win_h_target = rows_top + row_h * nvis + 6.0f * s;
    float win_t = ImGui::GetIO().DeltaTime * 10.0f;

    if (win_t > 1.0f)
    {
        win_t = 1.0f;
    }

    win_h_anim += (win_h_target - win_h_anim) * win_t;

    static bool opened = true;
    static bool pos_set = false;

    if (!pos_set)
    {
        float y = g_cfg.binds_y < 0.0f ? ImGui::GetIO().DisplaySize.y * 0.5f : g_cfg.binds_y;
        ImGui::SetNextWindowPos(ImVec2(g_cfg.binds_x, y));
        pos_set = true;
    }

    ImGui::SetNextWindowSize(ImVec2(win_w, win_h_anim));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin("##binds_window", &opened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing);

    ImDrawList* list = ImGui::GetWindowDrawList();
    list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

    ImVec2 window_pos = ImGui::GetWindowPos() + ImVec2(4.0f * s, 1.0f * s);
    float window_alpha = 255.0f * bind_alpha;
    float win_h = win_h_anim;

    static float save_cd = 0.0f;
    ImVec2 cur_pos = ImGui::GetWindowPos();

    if (fabsf(cur_pos.x - g_cfg.binds_x) > 0.5f || fabsf(cur_pos.y - g_cfg.binds_y) > 0.5f)
    {
        g_cfg.binds_x = cur_pos.x;
        g_cfg.binds_y = cur_pos.y;
        save_cd = 2.0f;
    }

    if (save_cd > 0.0f)
    {
        save_cd -= ImGui::GetIO().DeltaTime;

        if (save_cd <= 0.0f)
        {
            SaveGeneralConfig();
        }
    }

    Blur::Create(list, window_pos, window_pos + ImVec2(content_w, head_h), ImColor(255, 255, 255, (int)window_alpha), 4.0f * s, ImDrawCornerFlags_Top);

    const std::string title = tr("Бинды", "Binds");
    ImVec2 title_size = ImGui::CalcTextSize(title.c_str());
    float head_x = window_pos.x + (content_w - (16.0f * s + 6.0f * s + title_size.x)) * 0.5f;

    if (kb_texture)
    {
        list->AddImage((void*)kb_texture, ImVec2(head_x, window_pos.y + 8.0f * s), ImVec2(head_x + 16.0f * s, window_pos.y + 24.0f * s), ImVec2(0, 0), ImVec2(1, 1), g_cfg.accent.to_color().new_alpha((int)window_alpha).as_imcolor());
    }

    list->AddText(ImVec2(head_x + 22.0f * s, window_pos.y + 8.0f * s), c_color(255, 255, 255, (int)window_alpha).as_imcolor(), title.c_str());

    list->AddLine(window_pos + ImVec2(0, head_h - 1.0f * s), window_pos + ImVec2(content_w, head_h - 1.0f * s), c_color(255, 255, 255, (int)(12.75f * bind_alpha)).as_imcolor());

    Blur::Create(list, window_pos + ImVec2(0, head_h), window_pos + ImVec2(content_w, win_h), ImColor(100, 100, 100, (int)window_alpha), 4.0f * s, ImDrawCornerFlags_Bot);

    list->AddRect(window_pos, window_pos + ImVec2(content_w, win_h), c_color(100, 100, 100, (int)(100.0f * bind_alpha)).as_imcolor(), 4.0f * s);

    ImVec2 prev_pos = ImGui::GetCursorPos();
    float max_pos = 0.0f;

    for (int vi = 0; vi < nvis; vi++)
    {
        int i = vis[vi];
        std::string name = Bind_Display_Name(i);
        ImGui::SetCursorPos(ImVec2(pad_l, rows_top + max_pos));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, bind_alpha));
        ImGui::Text(name.c_str());
        ImGui::PopStyleColor();

        std::string bind_type = kBinds[i].bind->mode == 1 ? tr("[ холд ]", "[ hold ]") : tr("[ тогл ]", "[ toggled ]");
        float textsize = ImGui::CalcTextSize(bind_type.c_str()).x;
        float row_y = ImGui::GetItemRectMin().y;

        list->AddText(ImVec2(window_pos.x + content_w - pad_r - textsize, row_y), c_color(255, 255, 255, (int)(102.0f * bind_alpha)).as_imcolor(), bind_type.c_str());

        max_pos += row_h;
    }

    ImGui::SetCursorPos(prev_pos);

    list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

    ImGui::End(false);
    ImGui::PopStyleColor();
    ImGui::PopFont();
}
