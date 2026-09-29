#include "Menu/Menu.hpp"

#include "Game/Features.h"

void Menu::DrawMisc()
{
        static int sub3 = 0;
        static float skin_fade = 0.0f;
        static float run_fade = 0.0f;
        static float spd_fade = 0.0f;
        static float game_fade = 0.0f;
        static float aspect_fade = 0.0f;
        static float fov_fade = 0.0f;
        static float cam_fade = 0.0f;

        std::vector<std::string> items = { tr("Игрок", "Player"), tr("Движение", "Movement"), tr("Машина", "Vehicle") };
        DrawSubTabs(sub3, items);
        UpdateSubFade(3, sub3);
        ImGui::SetCursorPosX(0);
        ImGui::BeginChild("subtab_misc", ImVec2(), false);
        if (sub3 == 0)
        {
            ImGui::Columns(2, NULL, false);
            ImGui::SetColumnOffset(1, 270.0f * GetScale());
            GroupBegin(tr("Игрок", "Player"));
            if (Button(tr("Отхил", "Heal")))
            {
                CPed* pPedSelf = FindPlayerPed();

                if (pPedSelf)
                {
                    pPedSelf->m_fHealth = 200.0f;
                }
            }
            if (Button(tr("Броня", "Armor")))
            {
                CPed* pPedSelf = FindPlayerPed();

                if (pPedSelf)
                {
                    pPedSelf->m_fArmour = 350.0f;
                }
            }
            BindableCheckbox("airbreak", tr("Аирбрейк", "Airbreak"), &g_cfg.airbreake, &g_cfg.airbreake_bind);
            GroupEnd();
            ImGui::NextColumn();
            GroupBegin(tr("Защита", "Protection"));
            BindableCheckbox("godmode", tr("Годмод", "Godmode"), &g_cfg.godmode, &g_cfg.godmode_bind);
            BindableCheckbox("nofall", tr("Без урона от падения", "No fall damage"), &g_cfg.nofall, &g_cfg.nofall_bind);
            GroupEnd();
            GroupBegin(tr("Скин", "Skin"));
            BindableCheckbox("change_skin", tr("Поменять скин", "Change skin"), &g_cfg.changemodel, &g_cfg.changemodel_bind);
            CreateAnimation(skin_fade, g_cfg.changemodel, 0.3f, AnimLerp);
            if (g_cfg.changemodel || skin_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = skin_fade;
                if (InputInt(tr("ID скина", "Skin ID"), &g_cfg.playerSkinID))
                {
                    SaveGeneralConfig();
                }
                if (Button(tr("Применить", "Apply")))
                {
                    Skin::Update();
                }
                widget_alpha_mul = prev_mul;
            }
            GroupEnd();
        }
        else if (sub3 == 1)
        {
            ImGui::Columns(2, NULL, false);
            ImGui::SetColumnOffset(1, 270.0f * GetScale());
            GroupBegin(tr("Движение", "Movement"));
            BindableCheckbox("fast_run", tr("Быстрый бег", "Fast run"), &g_cfg.fastbeg, &g_cfg.fastbeg_bind);
            CreateAnimation(run_fade, g_cfg.fastbeg, 0.3f, AnimLerp);
            if (g_cfg.fastbeg || run_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = run_fade;
                if (SliderFloat(tr("Скорость", "Speed"), &g_cfg.fastbegs, 1.0f, 10.0f))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            BindableCheckbox("fastrot", tr("Быстрая ротация", "Fast Rotation"), &g_cfg.fastrot, &g_cfg.fastrot_bind);
            BindableCheckbox("nocamcol", tr("Без колизии камеры", "No camera collision"), &g_cfg.nocamcol, &g_cfg.nocamcol_bind);
            GroupEnd();
            ImGui::NextColumn();
            GroupBegin(tr("Мир", "World"));
            BindableCheckbox("gamespeed", tr("Скорость игры", "Game speed"), &g_cfg.gamespeed, &g_cfg.gamespeed_bind);
            CreateAnimation(game_fade, g_cfg.gamespeed, 0.3f, AnimLerp);
            if (g_cfg.gamespeed || game_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = game_fade;
                if (SliderFloat(tr("Темп", "Rate"), &g_cfg.gamespeedval, 0.1f, 3.0f))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            BindableCheckbox("aspect", tr("Аспект", "Aspect"), &g_cfg.aspect, &g_cfg.aspect_bind);
            CreateAnimation(aspect_fade, g_cfg.aspect, 0.3f, AnimLerp);
            if (g_cfg.aspect || aspect_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = aspect_fade;
                if (SliderFloat(tr("Соотношение сторон", "Aspect ratio"), &g_cfg.aspectval, 0.5f, 3.5f, "%.2f"))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            BindableCheckbox("fov", tr("Фов", "Fov"), &g_cfg.fov, &g_cfg.fov_bind);
            CreateAnimation(fov_fade, g_cfg.fov, 0.3f, AnimLerp);
            if (g_cfg.fov || fov_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = fov_fade;
                if (SliderFloat(tr("Поле зрения", "Field of view"), &g_cfg.fovval, 30.0f, 120.0f, "%.0f"))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            BindableCheckbox("camhack", tr("Камхак", "Camhack"), &g_cfg.camhack, &g_cfg.camhack_bind);
            CreateAnimation(cam_fade, g_cfg.camhack, 0.3f, AnimLerp);
            if (g_cfg.camhack || cam_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = cam_fade;
                if (SliderFloat(tr("Скорость", "Speed"), &g_cfg.camhackspeed, 0.01f, 2.0f, "%.2f"))
                {
                    SaveGeneralConfig();
                }
                if (Checkbox(tr("Телепорт при выходе", "Teleport on exit"), &g_cfg.camhackteleport))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            GroupEnd();
        }
        else
        {
            ImGui::Columns(2, NULL, false);
            ImGui::SetColumnOffset(1, 270.0f * GetScale());
            GroupBegin(tr("Машина", "Vehicle"));
            if (Button(tr("Починка", "Repair")))
            {
                Repair::Update();
            }
            if (Button(tr("Перевернуть", "Flip over")))
            {
                Flip::Update();
            }
            BindableCheckbox("speedhack", tr("Спидхак", "Speedhack"), &g_cfg.speedhack, &g_cfg.speedhack_bind);
            CreateAnimation(spd_fade, g_cfg.speedhack, 0.3f, AnimLerp);
            if (g_cfg.speedhack || spd_fade > 0.02f)
            {
                const float prev_mul = widget_alpha_mul;
                widget_alpha_mul = spd_fade;
                if (SliderFloat(tr("Мощность", "Power"), &g_cfg.MaxSpd, 0.0f, 30.0f, "%.1f"))
                {
                    SaveGeneralConfig();
                }
                widget_alpha_mul = prev_mul;
            }
            GroupEnd();
            ImGui::NextColumn();
            GroupBegin(tr("Авто", "Auto"));
            BindableCheckbox("autoengine", tr("Автозавод", "Auto engine"), &g_cfg.autoengine, &g_cfg.autoengine_bind);
            BindableCheckbox("autounlock", tr("Автооткрытие", "Auto unlock"), &g_cfg.autounlock, &g_cfg.autounlock_bind);
            GroupEnd();
            GroupBegin(tr("Трюки", "Stunts"));
            BindableCheckbox("nobikefall", tr("Не падать с байка", "No bike fall"), &g_cfg.nobikefall, &g_cfg.nobikefall_bind);
            BindableCheckbox("waterdrive", tr("Машины по воде", "Drive on water"), &g_cfg.waterdrive, &g_cfg.waterdrive_bind);
            BindableCheckbox("carfly", tr("Летающие машины", "Flying cars"), &g_cfg.carfly, &g_cfg.carfly_bind);
            GroupEnd();
        }
        ImGui::EndChild(false);
}
