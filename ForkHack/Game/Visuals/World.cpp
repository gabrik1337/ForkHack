#include "Game/Features.h"

#include "Game/Visuals/World.hpp"

#include "CClock.h"
#include "CVehicleModelInfo.h"
#include "CWeather.h"

#include <cctype>
#include <cfloat>
#include <cstdio>
#include <fstream>
#include <string>

void World::Update()
{
    static bool nightWas = false;
    static unsigned char nightH = 0;
    static unsigned char nightM = 0;
    static unsigned char nightD = 0;
    static bool rateWas = false;
    static unsigned int rateSaved = 60000;

    if (g_cfg.nightmode)
    {
        if (!nightWas)
        {
            nightH = CClock::ms_nGameClockHours;
            nightM = CClock::ms_nGameClockMinutes;
            nightD = CClock::ms_nGameClockDays;
            nightWas = true;
        }

        CClock::ms_nGameClockHours = 0;
        CClock::ms_nGameClockMinutes = 0;
    }
    else
    {
        if (nightWas)
        {
            CClock::ms_nGameClockHours = nightH;
            CClock::ms_nGameClockMinutes = nightM;
            CClock::ms_nGameClockDays = nightD;
            nightWas = false;
        }

        if (g_cfg.customtime)
        {
            CClock::ms_nGameClockHours = (unsigned char)std::clamp((int)g_cfg.timehour, 0, 23);
            CClock::ms_nGameClockMinutes = 0;

            if (g_cfg.freezetime)
            {
                if (!rateWas)
                {
                    rateSaved = CClock::ms_nMillisecondsPerGameMinute;
                    rateWas = true;
                }

                CClock::ms_nGameClockSeconds = 0;
                CClock::ms_nMillisecondsPerGameMinute = 3600000;
            }
            else if (rateWas)
            {
                CClock::ms_nMillisecondsPerGameMinute = rateSaved;
                rateWas = false;
            }
        }
        else if (rateWas)
        {
            CClock::ms_nMillisecondsPerGameMinute = rateSaved;
            rateWas = false;
        }
    }

    static bool weatherWas = false;

    if (g_cfg.weather > 0)
    {
        int wt = g_cfg.weather - 1;
        CWeather::ForceWeatherNow((short)wt);
        CWeather::ForcedWeatherType = (short)wt;
        weatherWas = true;
    }
    else if (weatherWas)
    {
        CWeather::ForcedWeatherType = -1;
        CWeather::SetWeatherToAppropriateTypeNow();
        weatherWas = false;
    }

    Timecycle::Update();
}
