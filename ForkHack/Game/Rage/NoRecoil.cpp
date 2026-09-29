#include "Game/Features.h"

#include "Game/Rage/NoRecoil.hpp"

void NoRecoil::Update()
{
    static bool applied = false;

    if (g_cfg.norecoil == applied)
    {
        return;
    }

    applied = g_cfg.norecoil;

    const unsigned char on[] = { 0xBE, 0x00, 0x00, 0x00, 0x00 };
    const unsigned char off[] = { 0xBE, 0x00, 0x00, 0x40, 0x3F };
    PatchBytes(reinterpret_cast<void*>(0x008D610F), applied ? on : off, 5);
}
