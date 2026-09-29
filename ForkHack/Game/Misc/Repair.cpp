#include "Game/Features.h"

#include "Game/Misc/Repair.hpp"

#include "CAutomobile.h"

void Repair::Update()
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

    veh->m_fHealth = 1000.0f;

    if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_AUTOMOBILE)
    {
        CAutomobile* autoVeh = reinterpret_cast<CAutomobile*>(veh);

        for (int i = 0; i < 4; i++)
        {
            autoVeh->FixTyre((eWheels)i);
        }
    }
}
