#include "Game/Features.h"

#include "Game/Misc/Vehicle/Speedhack.hpp"

#include <windows.h>

#include <algorithm>

void Speedhack::Update()
{
    if (!g_cfg.speedhack || g_cfg.airbreake)
    {
        return;
    }

    static DWORD lastTick = 0;
    DWORD now = GetTickCount();

    if (now - lastTick < 50)
    {
        return;
    }

    lastTick = now;

    CPed* pPedSelf = FindPlayerPed();

    if (!pPedSelf)
    {
        return;
    }

    CVehicle* veh = pPedSelf->m_pVehicle;

    if (!veh || veh->m_pDriver != pPedSelf)
    {
        return;
    }

    CVector& vel = veh->m_vecMoveSpeed;
    float direction = VecLength(vel);

    if (direction <= 0.01f)
    {
        return;
    }

    float power = std::clamp(g_cfg.MaxSpd, 0.0f, 30.0f);
    float boost = power / 180.0f;
    vel.x += (vel.x / direction) * boost;
    vel.y += (vel.y / direction) * boost;
    vel.z += (vel.z / direction) * boost;
}
