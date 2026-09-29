#include "Game/Features.h"

#include "Game/Misc/Flip.hpp"

#include <cmath>

void Flip::Update()
{
    CPed* pPedSelf = FindPlayerPed();

    if (!pPedSelf)
    {
        return;
    }

    CVehicle* veh = pPedSelf->m_pVehicle;

    if (!veh)
    {
        return;
    }

    CVector fwd = veh->GetForward();
    fwd.z = 0.0f;
    float len = sqrtf(fwd.x * fwd.x + fwd.y * fwd.y);

    if (len < 0.001f)
    {
        fwd.x = 0.0f;
        fwd.y = 1.0f;
    }
    else
    {
        fwd.x /= len;
        fwd.y /= len;
    }

    veh->GetRight() = CVector(fwd.y, -fwd.x, 0.0f);
    veh->GetForward() = fwd;
    veh->GetUp() = CVector(0.0f, 0.0f, 1.0f);
    veh->m_vecMoveSpeed.x = 0.0f;
    veh->m_vecMoveSpeed.y = 0.0f;
    veh->m_vecMoveSpeed.z = 0.0f;
    veh->m_vecTurnSpeed.x = 0.0f;
    veh->m_vecTurnSpeed.y = 0.0f;
    veh->m_vecTurnSpeed.z = 0.0f;
}
