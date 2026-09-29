#include "Game/Features.h"

#include "Game/Misc/NoFall.hpp"

namespace
{
    bool fallOn = false;
    unsigned char fallOrig[3] = {};
    bool fallHasOrig = false;
}

void NoFall::Update()
{
    if (!g_cfg.nofall)
    {
        if (fallOn)
        {
            if (fallHasOrig)
            {
                PatchBytes(reinterpret_cast<void*>(0x005E2D5D), fallOrig, 3);
                fallHasOrig = false;
            }

            fallOn = false;
        }

        return;
    }

    if (!fallOn)
    {
        if (!fallHasOrig && *reinterpret_cast<unsigned char*>(0x005E2D5D) != 0x90)
        {
            memcpy(fallOrig, reinterpret_cast<void*>(0x005E2D5D), 3);
            fallHasOrig = true;
        }

        static const unsigned char nop[3] = { 0x90, 0x90, 0x90 };
        PatchBytes(reinterpret_cast<void*>(0x005E2D5D), nop, 3);
        fallOn = true;
    }
}
