#include "Game/Features.h"

#include "Game/Rage/RapidFire.hpp"

void RapidFire::Update()
{
    static bool wasOn = false;
    static float snap[17][4][6] = {};

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

    if (!g_cfg.rapidfire)
    {
        if (wasOn)
        {
            each([&](CWeaponInfo* wi, int ti, int si)
            {
                float* f = reinterpret_cast<float*>(&wi->m_fAnimLoopStart);

                for (int k = 0; k < 6; k++)
                {
                    f[k] = snap[ti][si][k];
                }
            });
            wasOn = false;
        }

        return;
    }

    if (!wasOn)
    {
        each([&](CWeaponInfo* wi, int ti, int si)
        {
            float* f = reinterpret_cast<float*>(&wi->m_fAnimLoopStart);

            for (int k = 0; k < 6; k++)
            {
                snap[ti][si][k] = f[k];
            }
        });
        wasOn = true;
    }

    each([](CWeaponInfo* wi, int, int)
    {
        float* f = reinterpret_cast<float*>(&wi->m_fAnimLoopStart);

        for (int k = 0; k < 6; k++)
        {
            f[k] = 0.0f;
        }
    });
}
