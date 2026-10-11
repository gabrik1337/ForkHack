#include "Game/Features.h"

#include "Game/Misc/Vehicle/FastExit.hpp"

#include "CCarEnterExit.h"

#include <windows.h>

void FastExit::Trigger()
{
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

    CCarEnterExit::MakeUndraggedDriverPedLeaveCar(veh, pPedSelf);
}

void FastExit::Update()
{
    if (g_cfg.fastexit_bind.key < 0)
    {
        return;
    }

    if (!(GetAsyncKeyState(g_cfg.fastexit_bind.key) & 1))
    {
        return;
    }

    Trigger();
}
