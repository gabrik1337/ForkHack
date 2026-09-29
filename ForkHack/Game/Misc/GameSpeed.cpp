#include "Game/Features.h"

#include "Game/Misc/GameSpeed.hpp"

void GameSpeed::Update()
{
    static bool wasOn = false;

    if (!g_cfg.gamespeed)
    {
        if (wasOn)
        {
            CTimer::ms_fTimeScale = 1.0f;
            wasOn = false;
        }

        return;
    }

    wasOn = true;
    CTimer::ms_fTimeScale = std::clamp(g_cfg.gamespeedval, 0.1f, 3.0f);
}
