#include "Game/Features.h"

#include "Game/Misc/KeyBinds.hpp"

#include <map>

namespace
{
    void ProcessBind(keybind_t& bind, bool& value)
    {
    if (bind.key < 0 || bind.manual)
    {
        return;
    }

    bool down = (GetAsyncKeyState(bind.key) & 0x8000) != 0;

    if (bind.mode == 1)
    {
        value = down;
        return;
    }

    static std::map<int, bool> prev;
    bool was = prev[bind.key];

    if (down && !was)
    {
        value = !value;
    }

    prev[bind.key] = down;
}
}

void KeyBinds::Update()
{
    ProcessBind(g_cfg.godmode_bind, g_cfg.godmode);
    ProcessBind(g_cfg.nofall_bind, g_cfg.nofall);
    ProcessBind(g_cfg.fastbeg_bind, g_cfg.fastbeg);
    ProcessBind(g_cfg.wh_bind, g_cfg.wh);
    ProcessBind(g_cfg.speedhack_bind, g_cfg.speedhack);
    ProcessBind(g_cfg.airbreake_bind, g_cfg.airbreake);
    ProcessBind(g_cfg.changemodel_bind, g_cfg.changemodel);
    ProcessBind(g_cfg.nightmode_bind, g_cfg.nightmode);
    ProcessBind(g_cfg.customtime_bind, g_cfg.customtime);
    ProcessBind(g_cfg.freezetime_bind, g_cfg.freezetime);
    ProcessBind(g_cfg.suncolor_bind, g_cfg.suncolor);
    ProcessBind(g_cfg.customcolor_bind, g_cfg.customcolor);
    ProcessBind(g_cfg.fullbright_bind, g_cfg.fullbright);
    ProcessBind(g_cfg.rapidfire_bind, g_cfg.rapidfire);
    ProcessBind(g_cfg.infammo_bind, g_cfg.infammo);
    ProcessBind(g_cfg.gamespeed_bind, g_cfg.gamespeed);
    ProcessBind(g_cfg.fastrot_bind, g_cfg.fastrot);
    ProcessBind(g_cfg.nobikefall_bind, g_cfg.nobikefall);
    ProcessBind(g_cfg.waterdrive_bind, g_cfg.waterdrive);
    ProcessBind(g_cfg.carfly_bind, g_cfg.carfly);
    ProcessBind(g_cfg.autoengine_bind, g_cfg.autoengine);
    ProcessBind(g_cfg.autounlock_bind, g_cfg.autounlock);
    ProcessBind(g_cfg.fastcross_bind, g_cfg.fastcross);
    ProcessBind(g_cfg.norecoil_bind, g_cfg.norecoil);
    ProcessBind(g_cfg.nospread_bind, g_cfg.nospread);
    ProcessBind(g_cfg.trigger_bind, g_cfg.trigger);
    ProcessBind(g_cfg.nocamcol_bind, g_cfg.nocamcol);
    ProcessBind(g_cfg.aspect_bind, g_cfg.aspect);
    ProcessBind(g_cfg.fov_bind, g_cfg.fov);
    ProcessBind(g_cfg.camhack_bind, g_cfg.camhack);
    ProcessBind(g_cfg.camhack_bind, g_cfg.camhack);
    ProcessBind(g_cfg.nocol_bind, g_cfg.nocol);
    ProcessBind(g_cfg.removals_bind, g_cfg.removals);
    ProcessBind(g_cfg.skychange_bind, g_cfg.skychange);
    ProcessBind(g_cfg.infammo_bind, g_cfg.infammo);
}
