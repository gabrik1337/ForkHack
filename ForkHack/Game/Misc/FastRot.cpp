#include "Game/Features.h"

#include "Game/Misc/FastRot.hpp"

void FastRot::Update()
{
    static bool wasOn = false;
    static float saved = 0.0f;

    CPed* pPedSelf = FindPlayerPed();

    if (!pPedSelf)
    {
        return;
    }

    float* pRot = reinterpret_cast<float*>((DWORD)pPedSelf + 0x560);

    if (!g_cfg.fastrot)
    {
        if (wasOn)
        {
            *pRot = saved;
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        saved = *pRot;
        wasOn = true;
    }

    *pRot = 100.0f;
}
