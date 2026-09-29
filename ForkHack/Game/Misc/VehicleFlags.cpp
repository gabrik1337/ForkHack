#include "Game/Features.h"

#include "Game/Misc/VehicleFlags.hpp"

void VehicleFlags::Update()
{
    *(DWORD*)0x969152 = g_cfg.waterdrive ? 1 : 0;
    *(DWORD*)0x969160 = g_cfg.carfly ? 1 : 0;
}
