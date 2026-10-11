#include "Game/Features.h"

#include "Game/Visuals/Fov.hpp"

void Fov::Update()
{
    static bool wasOn = false;
    static float saved = 70.0f;
    static float savedCam[3] = { 70.0f, 70.0f, 70.0f };

    float* pFov = reinterpret_cast<float*>(0x8D5038);
    bool aiming = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    if (!g_cfg.fov || aiming)
    {
        if (wasOn)
        {
            *pFov = saved;

            for (int i = 0; i < 3; i++)
            {
                TheCamera.m_aCams[i].m_fFOV = savedCam[i];
            }

            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        saved = *pFov;

        for (int i = 0; i < 3; i++)
        {
            savedCam[i] = TheCamera.m_aCams[i].m_fFOV;
        }

        wasOn = true;
    }

    float v = std::clamp(g_cfg.fovval, 30.0f, 120.0f);
    *pFov = v;

    for (int i = 0; i < 3; i++)
    {
        TheCamera.m_aCams[i].m_fFOV = v;
    }
}
