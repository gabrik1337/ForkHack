#include "Game/Features.h"

#include "Game/Misc/AirBreak.hpp"

#include <cmath>

namespace
{
    bool airBrake = false;
}

void AirBreak::Run()
{
    for (;;)
    {
        if (g_cfg.nocamcol)
        {
            TheCamera.m_bMoveCamToAvoidGeom = false;
        }

        if (g_cfg.fov && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
        {
            float v = std::clamp(g_cfg.fovval, 30.0f, 120.0f);
            *(float*)0x8D5038 = v;
            TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fFOV = v;
        }

        if (!GetModuleHandleA("client.dll"))
        {
            Sleep(100);
            continue;
        }

        static float coordsXforPed = 0.0f;
        static float coordsYforPed = 0.0f;
        static float coordsZforPed = 0.0f;
        static float coordsXforCar = 0.0f;
        static float coordsYforCar = 0.0f;
        static float coordsZforCar = 0.0f;
        static float airSpeed = 0.05f;
        static bool airBrakeWasEnabled = false;

        if (g_cfg.airbreake && !airBrakeWasEnabled)
        {
            if (*(DWORD*)(0xBA18FC) == 0)
            {
                DWORD PEDSELF = *(DWORD*)0xB6F5F0;
                DWORD MyMatrix = *(DWORD*)(PEDSELF + 0x14);
                coordsXforPed = *(float*)(MyMatrix + 0x30);
                coordsYforPed = *(float*)(MyMatrix + 0x34);
                coordsZforPed = *(float*)(MyMatrix + 0x38);
            }
            else
            {
                DWORD CARSELF = *(DWORD*)0xBA18FC;
                DWORD car = *(DWORD*)(CARSELF + 0x14);
                coordsXforCar = *(float*)(car + 0x30);
                coordsYforCar = *(float*)(car + 0x34);
                coordsZforCar = *(float*)(car + 0x38);
            }
        }

        airBrake = g_cfg.airbreake;
        airBrakeWasEnabled = g_cfg.airbreake;

        if (GetAsyncKeyState(VK_LSHIFT) && airBrake && g_cfg.airbreake)
        {
            airSpeed += 0.02f;
            Sleep(100);
        }

        if (GetAsyncKeyState(VK_LCONTROL) && airBrake && g_cfg.airbreake)
        {
            airSpeed -= 0.02f;

            if (airSpeed < 0.05f)
            {
                airSpeed = 0.05f;
            }

            Sleep(100);
        }

        if (airBrake && g_cfg.airbreake)
        {
            float angle = *(float*)(0xB6F258);
            float airBrkX = 0.0f, airBrkY = 0.0f, airBrkZ = 0.0f;

            if (GetAsyncKeyState(0x57))
            {
                airBrkX = airBrkX - airSpeed * sin(-(angle - 1.5708));
                airBrkY = airBrkY - airSpeed * cos(-(angle - 1.5708));
            }

            if (GetAsyncKeyState(0x53))
            {
                airBrkX = airBrkX + airSpeed * sin(-(angle - 1.5708));
                airBrkY = airBrkY + airSpeed * cos(-(angle - 1.5708));
            }

            if (GetAsyncKeyState(0x41))
            {
                airBrkX = airBrkX - airSpeed * sin(-angle);
                airBrkY = airBrkY - airSpeed * cos(-angle);
            }

            if (GetAsyncKeyState(0x44))
            {
                airBrkX = airBrkX + airSpeed * sin(-angle);
                airBrkY = airBrkY + airSpeed * cos(-angle);
            }

            if (GetAsyncKeyState(VK_UP))
            {
                airBrkZ += airSpeed;
            }

            if (GetAsyncKeyState(VK_DOWN))
            {
                airBrkZ -= airSpeed;
            }

            if (*(DWORD*)(0xBA18FC) > 0)
            {
                DWORD CARSELF = *(DWORD*)0xBA18FC;
                DWORD car = *(DWORD*)(CARSELF + 0x14);

                *(float*)(CARSELF + 0x44) = 0;
                *(float*)(CARSELF + 0x48) = 0;
                *(float*)(CARSELF + 0x4C) = 0;

                coordsXforCar += airBrkX;
                coordsYforCar += airBrkY;
                coordsZforCar += airBrkZ;

                *(float*)(car + 0x30) = coordsXforCar;
                *(float*)(car + 0x34) = coordsYforCar;
                *(float*)(car + 0x38) = coordsZforCar;

                *(float*)(CARSELF + 0x50) = 0;
                *(float*)(CARSELF + 0x54) = 0;
                *(float*)(CARSELF + 0x58) = 0;

                *(float*)(car + 0x0) = sin(-(angle));
                *(float*)(car + 0x4) = cos(-(angle));
                *(float*)(car + 0x8) = 0;

                *(float*)(car + 0x10) = sin(-(angle + 1.5708));
                *(float*)(car + 0x14) = cos(-(angle + 1.5708));
                *(float*)(car + 0x18) = 0;
            }

            if (*(DWORD*)(0xBA18FC) == 0)
            {
                DWORD PEDSELF = *(DWORD*)0xB6F5F0;
                DWORD MyMatrix = *(DWORD*)(PEDSELF + 0x14);

                *(float*)(PEDSELF + 0x44) = 0;
                *(float*)(PEDSELF + 0x48) = 0;
                *(float*)(PEDSELF + 0x4C) = 0;

                *(BYTE*)(PEDSELF + 0x46C) = 3;

                coordsXforPed += airBrkX;
                coordsYforPed += airBrkY;
                coordsZforPed += airBrkZ;

                *(float*)(MyMatrix + 0x30) = coordsXforPed;
                *(float*)(MyMatrix + 0x34) = coordsYforPed;
                *(float*)(MyMatrix + 0x38) = coordsZforPed;
            }
        }

        Sleep(1);
    }
}
