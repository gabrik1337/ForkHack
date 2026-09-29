#include "Game/Features.h"

#include "Game/Visuals/NoCamCol.hpp"

void NoCamCol::Update()
{
    static bool wasOn = false;
    static bool saved = true;

    if (!g_cfg.nocamcol)
    {
        if (wasOn)
        {
            TheCamera.m_bMoveCamToAvoidGeom = saved;
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        saved = TheCamera.m_bMoveCamToAvoidGeom;
        wasOn = true;
    }

    TheCamera.m_bMoveCamToAvoidGeom = false;
}
