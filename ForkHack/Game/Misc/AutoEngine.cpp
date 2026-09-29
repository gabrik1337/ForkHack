#include "Game/Features.h"

#include "Game/Misc/AutoEngine.hpp"

void AutoEngine::Update()
{
    if (!g_cfg.autoengine)
    {
        return;
    }

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

    using tSetEngineOn = void(__thiscall*)(void*, bool);
    static tSetEngineOn SetEngineOn = reinterpret_cast<tSetEngineOn>(0x41BDD0);
    SetEngineOn(veh, true);
}
