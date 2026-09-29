#include "Game/Features.h"

#include "Game/Misc/AutoUnlock.hpp"

void AutoUnlock::Update()
{
    if (!g_cfg.autounlock)
    {
        return;
    }

    CPed* pPedSelf = FindPlayerPed();

    if (!pPedSelf || !CPools::ms_pVehiclePool)
    {
        return;
    }

    const CVector localPos = pPedSelf->GetPosition();
    const int poolSize = CPools::ms_pVehiclePool->m_nSize;

    for (int i = 0; i < poolSize; i++)
    {
        CVehicle* veh = CPools::ms_pVehiclePool->GetAt(i);

        if (!veh)
        {
            continue;
        }

        const CVector vehPos = veh->GetPosition();

        const float dx = vehPos.x - localPos.x;
        const float dy = vehPos.y - localPos.y;
        const float dz = vehPos.z - localPos.z;

        if (dx * dx + dy * dy + dz * dz > 30.0f * 30.0f)
        {
            continue;
        }

        veh->m_eDoorLock = DOORLOCK_UNLOCKED;
    }
}
