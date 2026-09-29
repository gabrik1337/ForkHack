#include "Game/Features.h"

#include "Game/Visuals/Fov.hpp"

void Fov::Update()
{
    static bool wasOn = false;
    static float saved = 70.0f;
    static float savedCam = 70.0f;

    float* pFov = reinterpret_cast<float*>(0x8D5038);
    CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
    bool aiming = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    if (!g_cfg.fov || aiming)
    {
        if (wasOn)
        {
            *pFov = saved;
            cam.m_fFOV = savedCam;
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        saved = *pFov;
        savedCam = cam.m_fFOV;
        wasOn = true;
    }

    float v = std::clamp(g_cfg.fovval, 30.0f, 120.0f);
    *pFov = v;
    cam.m_fFOV = v;
}
