#include "Game/Features.h"

#include "Game/Rage/NoSpread.hpp"

void NoSpread::Update()
{
    static bool wasOn = false;
    static int snap[17][4] = {};

    auto each = [](auto fn)
    {
        for (int t = 22; t <= 38; t++)
        {
            for (int s = 0; s < 4; s++)
            {
                if (CWeaponInfo* wi = CWeaponInfo::GetWeaponInfo((eWeaponType)t, (unsigned char)s))
                {
                    fn(wi, t - 22, s);
                }
            }
        }
    };

    if (!g_cfg.nospread)
    {
        if (wasOn)
        {
            each([&](CWeaponInfo* wi, int ti, int si)
            {
                *reinterpret_cast<int*>(&wi->m_fAccuracy) = snap[ti][si];
            });
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        each([&](CWeaponInfo* wi, int ti, int si)
        {
            snap[ti][si] = *reinterpret_cast<int*>(&wi->m_fAccuracy);
        });
        wasOn = true;
    }

    each([](CWeaponInfo* wi, int, int)
    {
        *reinterpret_cast<int*>(&wi->m_fAccuracy) = 1265353216;
    });
}
