#include "Menu/Menu.hpp"

#include "Game/Features.h"

#include "Gfx/Fonts.hpp"

#include <shellapi.h>

void Menu::DrawProfile()
{
        const float s = GetScale();
        static int sub4 = 0;
        static bool cfg_scanned = false;

        if (!cfg_scanned)
        {
            config_t::RefreshList();
            cfg_scanned = true;
        }

        std::vector<std::string> items = { tr("Конфиг", "Config"), tr("Настройки", "Settings"), tr("Важное", "Important") };
        DrawSubTabs(sub4, items);
        UpdateSubFade(4, sub4);
        ImGui::SetCursorPosX(0);
        ImGui::BeginChild("subtab_cfg", ImVec2(), false);
        ImGui::Columns(2, NULL, false);
        ImGui::SetColumnOffset(1, 270.0f * GetScale());

        if (sub4 == 0)
        {
            const bool wrong_config = g_cfg.cfg_list.empty() || g_cfg.cfg_selected < 0 || g_cfg.cfg_selected >= (int)g_cfg.cfg_list.size();
            const char* selected_cfg = wrong_config ? nullptr : g_cfg.cfg_list[g_cfg.cfg_selected].c_str();

            GroupBegin(tr("Общее", "General"));
            Textbox(tr("Имя конфига", "Config name"), g_cfg.cfg_name, sizeof(g_cfg.cfg_name));
            if (Button(tr("Создать", "Create")))
            {
                if (g_cfg.cfg_name[0] != 0)
                {
                    bool exists = false;

                    for (const auto& name : g_cfg.cfg_list)
                    {
                        if (name == g_cfg.cfg_name)
                        {
                            exists = true;
                            break;
                        }
                    }

                    if (!exists)
                    {
                        g_cfg.cfg_list.emplace_back(g_cfg.cfg_name);
                        g_cfg.cfg_selected = (int)g_cfg.cfg_list.size() - 1;
                        g_cfg.Save(g_cfg.cfg_name);
                        g_cfg.cfg_name[0] = 0;
                    }
                }
            }
            if (Button(tr("Обновить", "Refresh")))
            {
                config_t::RefreshList();
            }
            if (Button(tr("Папка конфигов", "Configs folder")))
            {
                const std::string folder = config_t::ConfigDir();
                CreateDirectoryA(folder.c_str(), nullptr);

                if ((INT_PTR)ShellExecuteA(NULL, "open", folder.c_str(), NULL, NULL, SW_SHOWNORMAL) <= 32)
                {
                    MessageBoxA(NULL, tr("Не удалось открыть папку конфигов", "Failed to open configs folder"), "ForkHack", MB_OK | MB_ICONERROR);
                }
            }
            GroupEnd();

            std::string config_title = std::string(tr("Конфиг: ", "Config: ")) + (wrong_config ? tr("Нет", "None") : selected_cfg);
            GroupBegin(config_title.c_str());
            if (Button(tr("Загрузить", "Load")) && !wrong_config)
            {
                g_cfg.Load(selected_cfg);
            }
            if (Button(tr("Сохранить", "Save")) && !wrong_config)
            {
                g_cfg.Save(selected_cfg);
            }
            if (Button(tr("Сбросить", "Reset")) && !wrong_config)
            {
                std::string target = selected_cfg;
                const bool was_open = g_cfg.menu_open;
                std::vector<std::string> saved_list = g_cfg.cfg_list;
                const int saved_selected = g_cfg.cfg_selected;
                g_cfg = config_t();
                g_cfg.menu_open = was_open;
                g_cfg.cfg_list = saved_list;
                g_cfg.cfg_selected = saved_selected;
                g_cfg.Save(target);
            }
            if (Button(tr("Удалить", "Delete")) && !wrong_config)
            {
                g_cfg.Remove(selected_cfg);
            }
            GroupEnd();
            ImGui::NextColumn();

            std::string list_title = std::string(tr("Конфигов | ", "Configs | ")) + std::to_string((int)g_cfg.cfg_list.size()) + tr(" всего", " total");
            GroupBegin(list_title.c_str());
            Textbox(tr("Поиск", "Search"), g_cfg.cfg_search, sizeof(g_cfg.cfg_search));
            Listbox("configlist___", &g_cfg.cfg_selected, g_cfg.cfg_list, 12, g_cfg.cfg_search);
            GroupEnd();
        }
        else if (sub4 == 1)
        {
            GroupBegin(tr("Интерфейс", "Interface"));
            ColorPicker(tr("Акцент", "Accent"), g_cfg.accent);
            if (Checkbox(tr("Кейбинды", "Keybinds"), &g_cfg.showbinds))
            {
                SaveGeneralConfig();
            }

            static const std::string dpi_s[] = { "75%", "100%", "125%", "150%", "175%", "200%" };
            const char* dpi_items[] = { dpi_s[0].c_str(), dpi_s[1].c_str(), dpi_s[2].c_str(), dpi_s[3].c_str(), dpi_s[4].c_str(), dpi_s[5].c_str() };
            const int dpi_values[] = { 75, 100, 125, 150, 175, 200 };
            int dpi_selected = 1;

    for (int i = 0; i < 6; i++)
            {
                if (dpi_values[i] == g_cfg.ui_scale)
                {
                    dpi_selected = i;
                    break;
                }
            }

            if (Combo(tr("Масштаб DPI", "DPI scale"), &dpi_selected, dpi_items, 6))
            {
                g_cfg.ui_scale = dpi_values[dpi_selected];
            }

            static const std::string lang_s[] = { "Русский", "English" };
            const char* lang_items[] = { lang_s[0].c_str(), lang_s[1].c_str() };
            if (Combo(tr("Язык", "Language"), &g_cfg.language, lang_items, 2))
            {
                SaveGeneralConfig();
            }
            GroupEnd();
        }
        else
        {
            ImGui::Columns(1);
            ImGui::Dummy(ImVec2(0, 55.0f * s));

            const float ccx = (ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowContentRegionMax().x) * 0.5f;
            c_color accent = g_cfg.accent.to_color();

            static const std::string role_s = "Разработчик";
            static const std::string role_e = "Developer";
            const char* role = (g_cfg.language == 1 ? role_e : role_s).c_str();
            ImVec2 role_size = ImGui::CalcTextSize(role);
            ImGui::SetCursorPosX(ccx - role_size.x * 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f * alpha));
            ImGui::Text(role);
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 2.0f * s));

            static const std::string dev_s = "gabrik1337";
            ImGui::PushFont(g_fonts.dmg);
            ImVec2 nick_size = ImGui::CalcTextSize(dev_s.c_str());
            ImGui::SetCursorPosX(ccx - nick_size.x * 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
            ImGui::Text(dev_s.c_str());
            ImGui::PopStyleColor();
            ImVec2 nick_min = ImGui::GetItemRectMin();
            ImVec2 nick_max = ImGui::GetItemRectMax();
            const float ncx = (nick_min.x + nick_max.x) * 0.5f;
            ImGui::PopFont();

            draw_list->AddRectFilled(ImVec2(ncx - 30.0f * s, nick_max.y + 6.0f * s), ImVec2(ncx + 30.0f * s, nick_max.y + 8.0f * s), accent.new_alpha((int)(255 * alpha)).as_imcolor(), 1.0f * s);

            ImGui::Dummy(ImVec2(0, 12.0f * s));
            ImGui::SetCursorPosX(ccx - 128.0f * s);

            if (Button(tr("Контакты", "Contacts")))
            {
                ShellExecuteA(NULL, "open", "https://t.me/gbr1337xd", NULL, NULL, SW_SHOWNORMAL);
                ShellExecuteA(NULL, "open", "https://discord.com/users/1138590550635839488", NULL, NULL, SW_SHOWNORMAL);
            }
        }

        ImGui::EndChild(false);
}
