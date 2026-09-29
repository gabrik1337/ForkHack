#include "Game/Features.h"

#include "Game/Misc/FastRun.hpp"

void FastRun::ApplyRunSpeed(float speed)
{
    CPed* pPedSelf = FindPlayerPed();

    if (!pPedSelf)
    {
        return;
    }

    plugin::Command<0x0393>(pPedSelf, "WOMAN_RUN", speed);
    plugin::Command<0x0393>(pPedSelf, "WOMAN_RUNBUSY", speed);
    plugin::Command<0x0393>(pPedSelf, "WOMAN_RUNPANIC", speed);
    plugin::Command<0x0393>(pPedSelf, "WOMAN_RUNSEXY", speed);
    plugin::Command<0x0393>(pPedSelf, "SPRINT_CIVI", speed);
    plugin::Command<0x0393>(pPedSelf, "SPRINT_PANIC", speed);
    plugin::Command<0x0393>(pPedSelf, "SWAT_RUN", speed);
    plugin::Command<0x0393>(pPedSelf, "FATSPRINT", speed);
}

void FastRun::Update()
{
    if (g_cfg.fastbeg)
    {
        ApplyRunSpeed(g_cfg.fastbegs);
    }
    else
    {
        ApplyRunSpeed(1.0f);
    }
}
