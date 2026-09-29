#include "Game/Features.h"

#include "Game/Rage/FastCrosshair.hpp"

void FastCrosshair::Update()
{
    static bool applied = false;

    if (g_cfg.fastcross == applied)
    {
        return;
    }

    applied = g_cfg.fastcross;

    const unsigned char on[] = { 0xEB };
    const unsigned char off[] = { 0x74 };
    PatchBytes(reinterpret_cast<void*>(0x0058E1D9), applied ? on : off, 1);
}
