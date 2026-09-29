#include "Game/Features.h"

#include "Game/Misc/CamHack.hpp"

#include <cmath>

namespace
{
    bool camOn = false;
    unsigned char camOrig[3][6] = {};
    bool camSnapped = false;

    void CamPatch(bool on)
    {
        static const DWORD addrs[3] = { 0x0052C7C2, 0x0052C7D1, 0x0052C7C8 };
        static const unsigned char nop[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

        if (on)
        {
            if (!camSnapped)
            {
                for (int i = 0; i < 3; i++)
                {
                    memcpy(camOrig[i], reinterpret_cast<void*>(addrs[i]), 6);
                }

                camSnapped = true;
            }

            for (int i = 0; i < 3; i++)
            {
                PatchBytes(reinterpret_cast<void*>(addrs[i]), nop, 6);
            }
        }
        else if (camSnapped)
        {
            for (int i = 0; i < 3; i++)
            {
                PatchBytes(reinterpret_cast<void*>(addrs[i]), camOrig[i], 6);
            }

            camSnapped = false;
        }
    }
}

void CamHack::Update()
{
    if (!g_cfg.camhack)
    {
        if (camOn)
        {
            CamPatch(false);
            camOn = false;

            if (g_cfg.camhackteleport)
            {
                CPed* pPedSelf = FindPlayerPed();
                CVector dst = *(CVector*)0xB6F9CC;

                if (pPedSelf)
                {
                    if (CVehicle* veh = pPedSelf->m_pVehicle)
                    {
                        veh->SetPosition(dst.x, dst.y, dst.z);
                        veh->m_vecMoveSpeed.x = 0.0f;
                        veh->m_vecMoveSpeed.y = 0.0f;
                        veh->m_vecMoveSpeed.z = 0.0f;
                    }
                    else
                    {
                        pPedSelf->SetPosition(dst.x, dst.y, dst.z);
                        pPedSelf->m_vecMoveSpeed.x = 0.0f;
                        pPedSelf->m_vecMoveSpeed.y = 0.0f;
                        pPedSelf->m_vecMoveSpeed.z = 0.0f;
                    }
                }
            }
        }

        return;
    }

    if (!camOn)
    {
        CamPatch(true);
        camOn = true;
    }

    CVector* camPos = reinterpret_cast<CVector*>(0xB6F9CC);
    float rot = *(float*)0xB6F178;
    float speed = std::clamp(g_cfg.camhackspeed, 0.01f, 2.0f);
    float sr = sinf(rot);
    float cr = cosf(rot);

    if (GetAsyncKeyState('W') & 0x8000) { camPos->x += sr * speed; camPos->y += cr * speed; }
    if (GetAsyncKeyState('S') & 0x8000) { camPos->x -= sr * speed; camPos->y -= cr * speed; }
    if (GetAsyncKeyState('A') & 0x8000) { camPos->x -= cr * speed; camPos->y += sr * speed; }
    if (GetAsyncKeyState('D') & 0x8000) { camPos->x += cr * speed; camPos->y -= sr * speed; }
    if (GetAsyncKeyState(VK_UP) & 0x8000) { camPos->z += speed; }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) { camPos->z -= speed; }
}
