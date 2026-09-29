#include "Game/Features.h"

#include "Game/Rage/InfAmmo.hpp"

void InfAmmo::Update()
{
    *(BYTE*)0x969178 = g_cfg.infammo ? 1 : 0;
}
