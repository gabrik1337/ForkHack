#include "Game/Features.h"

#include "Game/Visuals/Aspect.hpp"

void Aspect::Update()
{
    static bool wasOn = false;
    static float saved = 1.78f;

    float* pAspect = reinterpret_cast<float*>(0xC3EFA4);

    if (!g_cfg.aspect)
    {
        if (wasOn)
        {
            *pAspect = saved;
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        saved = *pAspect;
        wasOn = true;
    }

    *pAspect = std::clamp(g_cfg.aspectval, 0.5f, 3.5f);
}
