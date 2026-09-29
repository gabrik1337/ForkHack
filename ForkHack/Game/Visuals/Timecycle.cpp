#include "Game/Features.h"

#include "Game/Visuals/Timecycle.hpp"

#include "CTimeCycle.h"

#include <cstring>

namespace
{
    constexpr int TC_N = 184;
    constexpr int TC_SKY = 184;

    unsigned char bSkyTopR[TC_SKY]{}, bSkyTopG[TC_SKY]{}, bSkyTopB[TC_SKY]{};
    unsigned char bSkyBotR[TC_N]{}, bSkyBotG[TC_N]{}, bSkyBotB[TC_N]{};
    unsigned char bAmbR[TC_N]{}, bAmbG[TC_N]{}, bAmbB[TC_N]{};
    unsigned char bAmbObjR[TC_N]{}, bAmbObjG[TC_N]{}, bAmbObjB[TC_N]{};
    unsigned char bWaterR[TC_N]{}, bWaterG[TC_N]{}, bWaterB[TC_N]{};
    unsigned char bShadow[TC_N]{}, bLightShadow[TC_N]{}, bPoleShadow[TC_N]{};
    unsigned char bSunCoreR[TC_N]{}, bSunCoreG[TC_N]{}, bSunCoreB[TC_N]{};
    unsigned char bSunCoronaR[TC_N]{}, bSunCoronaG[TC_N]{}, bSunCoronaB[TC_N]{};
    unsigned char bCloudAlpha[TC_N]{};
    signed short bFogStart[TC_N]{};
    bool tc_backed = false;

    void SetTable(unsigned char* table, int n, unsigned char v)
    {
        for (int i = 0; i < n; i++)
        {
            table[i] = v;
        }
    }

    void SetTable(signed short* table, int n, signed short v)
    {
        for (int i = 0; i < n; i++)
        {
            table[i] = v;
        }
    }

    void SnapshotTimecycle()
    {
        memcpy(bSkyTopR, CTimeCycle::m_nSkyTopRed, sizeof(bSkyTopR));
        memcpy(bSkyTopG, CTimeCycle::m_nSkyTopGreen, sizeof(bSkyTopG));
        memcpy(bSkyTopB, CTimeCycle::m_nSkyTopBlue, sizeof(bSkyTopB));
        memcpy(bSkyBotR, CTimeCycle::m_nSkyBottomRed, sizeof(bSkyBotR));
        memcpy(bSkyBotG, CTimeCycle::m_nSkyBottomGreen, sizeof(bSkyBotG));
        memcpy(bSkyBotB, CTimeCycle::m_nSkyBottomBlue, sizeof(bSkyBotB));
        memcpy(bAmbR, CTimeCycle::m_nAmbientRed, sizeof(bAmbR));
        memcpy(bAmbG, CTimeCycle::m_nAmbientGreen, sizeof(bAmbG));
        memcpy(bAmbB, CTimeCycle::m_nAmbientBlue, sizeof(bAmbB));
        memcpy(bAmbObjR, CTimeCycle::m_nAmbientRed_Obj, sizeof(bAmbObjR));
        memcpy(bAmbObjG, CTimeCycle::m_nAmbientGreen_Obj, sizeof(bAmbObjG));
        memcpy(bAmbObjB, CTimeCycle::m_nAmbientBlue_Obj, sizeof(bAmbObjB));
        memcpy(bWaterR, CTimeCycle::m_fWaterRed, sizeof(bWaterR));
        memcpy(bWaterG, CTimeCycle::m_fWaterGreen, sizeof(bWaterG));
        memcpy(bWaterB, CTimeCycle::m_fWaterBlue, sizeof(bWaterB));
        memcpy(bShadow, CTimeCycle::m_nShadowStrength, sizeof(bShadow));
        memcpy(bLightShadow, CTimeCycle::m_nLightShadowStrength, sizeof(bLightShadow));
        memcpy(bPoleShadow, CTimeCycle::m_nPoleShadowStrength, sizeof(bPoleShadow));
        memcpy(bSunCoreR, CTimeCycle::m_nSunCoreRed, sizeof(bSunCoreR));
        memcpy(bSunCoreG, CTimeCycle::m_nSunCoreGreen, sizeof(bSunCoreG));
        memcpy(bSunCoreB, CTimeCycle::m_nSunCoreBlue, sizeof(bSunCoreB));
        memcpy(bSunCoronaR, CTimeCycle::m_nSunCoronaRed, sizeof(bSunCoronaR));
        memcpy(bSunCoronaG, CTimeCycle::m_nSunCoronaGreen, sizeof(bSunCoronaG));
        memcpy(bSunCoronaB, CTimeCycle::m_nSunCoronaBlue, sizeof(bSunCoronaB));
        memcpy(bCloudAlpha, CTimeCycle::m_fCloudAlpha, sizeof(bCloudAlpha));
        memcpy(bFogStart, CTimeCycle::m_fFogStart, sizeof(bFogStart));
    }

    void RestoreTimecycle()
    {
        memcpy(CTimeCycle::m_nSkyTopRed, bSkyTopR, sizeof(bSkyTopR));
        memcpy(CTimeCycle::m_nSkyTopGreen, bSkyTopG, sizeof(bSkyTopG));
        memcpy(CTimeCycle::m_nSkyTopBlue, bSkyTopB, sizeof(bSkyTopB));
        memcpy(CTimeCycle::m_nSkyBottomRed, bSkyBotR, sizeof(bSkyBotR));
        memcpy(CTimeCycle::m_nSkyBottomGreen, bSkyBotG, sizeof(bSkyBotG));
        memcpy(CTimeCycle::m_nSkyBottomBlue, bSkyBotB, sizeof(bSkyBotB));
        memcpy(CTimeCycle::m_nAmbientRed, bAmbR, sizeof(bAmbR));
        memcpy(CTimeCycle::m_nAmbientGreen, bAmbG, sizeof(bAmbG));
        memcpy(CTimeCycle::m_nAmbientBlue, bAmbB, sizeof(bAmbB));
        memcpy(CTimeCycle::m_nAmbientRed_Obj, bAmbObjR, sizeof(bAmbObjR));
        memcpy(CTimeCycle::m_nAmbientGreen_Obj, bAmbObjG, sizeof(bAmbObjG));
        memcpy(CTimeCycle::m_nAmbientBlue_Obj, bAmbObjB, sizeof(bAmbObjB));
        memcpy(CTimeCycle::m_fWaterRed, bWaterR, sizeof(bWaterR));
        memcpy(CTimeCycle::m_fWaterGreen, bWaterG, sizeof(bWaterG));
        memcpy(CTimeCycle::m_fWaterBlue, bWaterB, sizeof(bWaterB));
        memcpy(CTimeCycle::m_nShadowStrength, bShadow, sizeof(bShadow));
        memcpy(CTimeCycle::m_nLightShadowStrength, bLightShadow, sizeof(bLightShadow));
        memcpy(CTimeCycle::m_nPoleShadowStrength, bPoleShadow, sizeof(bPoleShadow));
        memcpy(CTimeCycle::m_nSunCoreRed, bSunCoreR, sizeof(bSunCoreR));
        memcpy(CTimeCycle::m_nSunCoreGreen, bSunCoreG, sizeof(bSunCoreG));
        memcpy(CTimeCycle::m_nSunCoreBlue, bSunCoreB, sizeof(bSunCoreB));
        memcpy(CTimeCycle::m_nSunCoronaRed, bSunCoronaR, sizeof(bSunCoronaR));
        memcpy(CTimeCycle::m_nSunCoronaGreen, bSunCoronaG, sizeof(bSunCoronaG));
        memcpy(CTimeCycle::m_nSunCoronaBlue, bSunCoronaB, sizeof(bSunCoronaB));
        memcpy(CTimeCycle::m_fCloudAlpha, bCloudAlpha, sizeof(bCloudAlpha));
        memcpy(CTimeCycle::m_fFogStart, bFogStart, sizeof(bFogStart));
    }

    void WriteAmbColors(unsigned char r, unsigned char g, unsigned char b)
    {
        SetTable(CTimeCycle::m_nAmbientRed, TC_N, r);
        SetTable(CTimeCycle::m_nAmbientGreen, TC_N, g);
        SetTable(CTimeCycle::m_nAmbientBlue, TC_N, b);
        SetTable(CTimeCycle::m_nAmbientRed_Obj, TC_N, r);
        SetTable(CTimeCycle::m_nAmbientGreen_Obj, TC_N, g);
        SetTable(CTimeCycle::m_nAmbientBlue_Obj, TC_N, b);
    }

    void WriteSkyColors(unsigned char topR, unsigned char topG, unsigned char topB,
        unsigned char botR, unsigned char botG, unsigned char botB)
    {
        SetTable(CTimeCycle::m_nSkyTopRed, TC_SKY, topR);
        SetTable(CTimeCycle::m_nSkyTopGreen, TC_SKY, topG);
        SetTable(CTimeCycle::m_nSkyTopBlue, TC_SKY, topB);
        SetTable(CTimeCycle::m_nSkyBottomRed, TC_N, botR);
        SetTable(CTimeCycle::m_nSkyBottomGreen, TC_N, botG);
        SetTable(CTimeCycle::m_nSkyBottomBlue, TC_N, botB);
    }

    void WriteWaterColors(unsigned char r, unsigned char g, unsigned char b)
    {
        SetTable(CTimeCycle::m_fWaterRed, TC_N, r);
        SetTable(CTimeCycle::m_fWaterGreen, TC_N, g);
        SetTable(CTimeCycle::m_fWaterBlue, TC_N, b);
    }

    void WriteSunColors(unsigned char r, unsigned char g, unsigned char b)
    {
        SetTable(CTimeCycle::m_nSunCoreRed, TC_N, r);
        SetTable(CTimeCycle::m_nSunCoreGreen, TC_N, g);
        SetTable(CTimeCycle::m_nSunCoreBlue, TC_N, b);
        SetTable(CTimeCycle::m_nSunCoronaRed, TC_N, r);
        SetTable(CTimeCycle::m_nSunCoronaGreen, TC_N, g);
        SetTable(CTimeCycle::m_nSunCoronaBlue, TC_N, b);
    }
}

void Timecycle::Update()
{
    if (!CTimeCycle::m_nAmbientRed || !CTimeCycle::m_nSkyTopRed || !CTimeCycle::m_fFarClip)
    {
        return;
    }

    bool active = g_cfg.nightmode || g_cfg.customcolor || g_cfg.skychange || g_cfg.fullbright || g_cfg.suncolor || (g_cfg.removals && (g_cfg.removal_flags & (REM_SHADOWS | REM_SUN | REM_CLOUDS)));

    if (!active)
    {
        if (tc_backed)
        {
            RestoreTimecycle();
            tc_backed = false;
        }

        return;
    }

    if (!tc_backed)
    {
        SnapshotTimecycle();
        tc_backed = true;
    }

    if (g_cfg.skychange)
    {
        const c_color skyTop = g_cfg.skycol.to_color();
        const c_color skyBot = g_cfg.skybotcol.to_color();
        WriteSkyColors(skyTop.r(), skyTop.g(), skyTop.b(), skyBot.r(), skyBot.g(), skyBot.b());
    }
    else if (g_cfg.nightmode)
    {
        WriteSkyColors(8, 12, 30, 20, 30, 55);
    }
    else
    {
        memcpy(CTimeCycle::m_nSkyTopRed, bSkyTopR, sizeof(bSkyTopR));
        memcpy(CTimeCycle::m_nSkyTopGreen, bSkyTopG, sizeof(bSkyTopG));
        memcpy(CTimeCycle::m_nSkyTopBlue, bSkyTopB, sizeof(bSkyTopB));
        memcpy(CTimeCycle::m_nSkyBottomRed, bSkyBotR, sizeof(bSkyBotR));
        memcpy(CTimeCycle::m_nSkyBottomGreen, bSkyBotG, sizeof(bSkyBotG));
        memcpy(CTimeCycle::m_nSkyBottomBlue, bSkyBotB, sizeof(bSkyBotB));
    }

    if (g_cfg.nightmode)
    {
        WriteAmbColors(40, 40, 60);
    }
    else if (g_cfg.customcolor)
    {
        const c_color amb = g_cfg.ambcol.to_color();
        WriteAmbColors(amb.r(), amb.g(), amb.b());
    }
    else if (g_cfg.fullbright)
    {
        WriteAmbColors(255, 255, 255);
    }
    else
    {
        memcpy(CTimeCycle::m_nAmbientRed, bAmbR, sizeof(bAmbR));
        memcpy(CTimeCycle::m_nAmbientGreen, bAmbG, sizeof(bAmbG));
        memcpy(CTimeCycle::m_nAmbientBlue, bAmbB, sizeof(bAmbB));
        memcpy(CTimeCycle::m_nAmbientRed_Obj, bAmbObjR, sizeof(bAmbObjR));
        memcpy(CTimeCycle::m_nAmbientGreen_Obj, bAmbObjG, sizeof(bAmbObjG));
        memcpy(CTimeCycle::m_nAmbientBlue_Obj, bAmbObjB, sizeof(bAmbObjB));
    }

    if (g_cfg.customcolor)
    {
        const c_color water = g_cfg.watercol.to_color();
        WriteWaterColors(water.r(), water.g(), water.b());
    }
    else if (g_cfg.nightmode)
    {
        WriteWaterColors(12, 25, 50);
    }
    else
    {
        memcpy(CTimeCycle::m_fWaterRed, bWaterR, sizeof(bWaterR));
        memcpy(CTimeCycle::m_fWaterGreen, bWaterG, sizeof(bWaterG));
        memcpy(CTimeCycle::m_fWaterBlue, bWaterB, sizeof(bWaterB));
    }

    if (g_cfg.fullbright)
    {
        SetTable(CTimeCycle::m_fFogStart, TC_N, 9999);
    }
    else
    {
        memcpy(CTimeCycle::m_fFogStart, bFogStart, sizeof(bFogStart));
    }

    if (g_cfg.removals && (g_cfg.removal_flags & REM_SHADOWS))
    {
        SetTable(CTimeCycle::m_nShadowStrength, TC_N, 0);
        SetTable(CTimeCycle::m_nLightShadowStrength, TC_N, 0);
        SetTable(CTimeCycle::m_nPoleShadowStrength, TC_N, 0);
    }
    else
    {
        memcpy(CTimeCycle::m_nShadowStrength, bShadow, sizeof(bShadow));
        memcpy(CTimeCycle::m_nLightShadowStrength, bLightShadow, sizeof(bLightShadow));
        memcpy(CTimeCycle::m_nPoleShadowStrength, bPoleShadow, sizeof(bPoleShadow));
    }

    if (g_cfg.suncolor)
    {
        const c_color sun = g_cfg.suncol.to_color();
        WriteSunColors(sun.r(), sun.g(), sun.b());
    }
    else if (g_cfg.removals && (g_cfg.removal_flags & REM_SUN))
    {
        SetTable(CTimeCycle::m_nSunCoreRed, TC_N, 0);
        SetTable(CTimeCycle::m_nSunCoreGreen, TC_N, 0);
        SetTable(CTimeCycle::m_nSunCoreBlue, TC_N, 0);
        SetTable(CTimeCycle::m_nSunCoronaRed, TC_N, 0);
        SetTable(CTimeCycle::m_nSunCoronaGreen, TC_N, 0);
        SetTable(CTimeCycle::m_nSunCoronaBlue, TC_N, 0);
    }
    else
    {
        memcpy(CTimeCycle::m_nSunCoreRed, bSunCoreR, sizeof(bSunCoreR));
        memcpy(CTimeCycle::m_nSunCoreGreen, bSunCoreG, sizeof(bSunCoreG));
        memcpy(CTimeCycle::m_nSunCoreBlue, bSunCoreB, sizeof(bSunCoreB));
        memcpy(CTimeCycle::m_nSunCoronaRed, bSunCoronaR, sizeof(bSunCoronaR));
        memcpy(CTimeCycle::m_nSunCoronaGreen, bSunCoronaG, sizeof(bSunCoronaG));
        memcpy(CTimeCycle::m_nSunCoronaBlue, bSunCoronaB, sizeof(bSunCoronaB));
    }

    if (g_cfg.removals && (g_cfg.removal_flags & REM_CLOUDS))
    {
        SetTable(CTimeCycle::m_fCloudAlpha, TC_N, 0);
    }
    else
    {
        memcpy(CTimeCycle::m_fCloudAlpha, bCloudAlpha, sizeof(bCloudAlpha));
    }
}
