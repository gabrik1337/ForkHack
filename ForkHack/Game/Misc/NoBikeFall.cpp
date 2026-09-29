#include "Game/Features.h"

#include "Game/Misc/NoBikeFall.hpp"

namespace
{
    bool fallOff = false;
    unsigned char fallOrig[5] = {};
    bool fallHasOrig = false;
}

void NoBikeFall::Update()
{
    if (!g_cfg.nobikefall)
    {
        if (fallOff)
        {
            if (fallHasOrig)
            {
                PatchBytes(reinterpret_cast<void*>(0x004BA3B9), fallOrig, 5);
                fallHasOrig = false;
            }

            fallOff = false;
        }

        return;
    }

    if (!fallOff)
    {
        if (!fallHasOrig && *reinterpret_cast<unsigned char*>(0x004BA3B9) != 0xE9)
        {
            memcpy(fallOrig, reinterpret_cast<void*>(0x004BA3B9), 5);
            fallHasOrig = true;
        }

        static const unsigned char jmp[] = { 0xE9, 0xA7, 0x03, 0x00, 0x00 };
        PatchBytes(reinterpret_cast<void*>(0x004BA3B9), jmp, 5);
        fallOff = true;
    }
}
