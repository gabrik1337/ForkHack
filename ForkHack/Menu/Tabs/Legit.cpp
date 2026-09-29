#include "Menu/Menu.hpp"

#include "Game/Features.h"

void Menu::DrawLegit()
{
        ImGui::SetCursorPosX(0);
        ImGui::BeginChild("subtab_legit", ImVec2(), false);
        ImGui::Columns(2, NULL, false);
        ImGui::SetColumnOffset(1, 270.0f * GetScale());
        GroupBegin(tr("Триггербот", "Triggerbot"));
        BindableCheckbox("trigger", tr("Триггербот", "Triggerbot"), &g_cfg.trigger, &g_cfg.trigger_bind);
        GroupEnd();
        ImGui::EndChild(false);
}
