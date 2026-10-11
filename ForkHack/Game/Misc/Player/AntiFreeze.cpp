#include "Game/Features.h"

#include "Game/Misc/Player/AntiFreeze.hpp"

#include "CPad.h"

void AntiFreeze::Update()
{
    if (!g_cfg.antifreeze)
    {
        return;
    }

    if (CPad* pad0 = CPad::GetPad(0))
    {
        pad0->DisablePlayerControls = 0;
    }

    if (CPad* pad1 = CPad::GetPad(1))
    {
        pad1->DisablePlayerControls = 0;
    }
}
