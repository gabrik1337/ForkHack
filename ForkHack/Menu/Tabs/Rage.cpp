#include "Menu/Menu.hpp"

#include "Game/Features.h"

void Menu::DrawRage()
{
        static float nocol_fade = 0.0f;

        ImGui::SetCursorPosX(0);
        ImGui::BeginChild("subtab_rage", ImVec2(), false);
        ImGui::Columns(2, NULL, false);
        ImGui::SetColumnOffset(1, 270.0f * GetScale());
        GroupBegin(tr("Оружие", "Weapon"));
        BindableCheckbox("rapidfire", tr("Рапид", "Rapid fire"), &g_cfg.rapidfire, &g_cfg.rapidfire_bind);
        BindableCheckbox("infammo", tr("Бесконечные патроны", "Infinite ammo"), &g_cfg.infammo, &g_cfg.infammo_bind);
        BindableCheckbox("fastcross", tr("Быстрый прицел", "Fast crosshair"), &g_cfg.fastcross, &g_cfg.fastcross_bind);
        GroupEnd();
        ImGui::NextColumn();
        GroupBegin(tr("Точность", "Accuracy"));
        BindableCheckbox("norecoil", tr("Без отдачи", "No recoil"), &g_cfg.norecoil, &g_cfg.norecoil_bind);
        BindableCheckbox("nospread", tr("Без разброса", "No spread"), &g_cfg.nospread, &g_cfg.nospread_bind);
        GroupEnd();
        GroupBegin(tr("Анти-колизия", "Anti-collision"));
        BindableCheckbox("nocol", tr("Анти-колизия", "No collision"), &g_cfg.nocol, &g_cfg.nocol_bind);
        CreateAnimation(nocol_fade, g_cfg.nocol, 0.3f, AnimLerp);
        if (g_cfg.nocol || nocol_fade > 0.02f)
        {
            const float prev_mul = widget_alpha_mul;
            widget_alpha_mul = nocol_fade;
            std::vector<std::string> nocol_items = { tr("Машины", "Vehicles"), tr("Педы", "Peds"), tr("Объекты", "Objects") };
            const unsigned int prev_flags = g_cfg.nocol_flags;
            MultiCombo(tr("Игнорировать", "Ignore"), g_cfg.nocol_flags, nocol_items);
            if (prev_flags != g_cfg.nocol_flags)
            {
                SaveGeneralConfig();
            }
            widget_alpha_mul = prev_mul;
        }
        GroupEnd();
        ImGui::EndChild(false);
}
