#include "Game/Features.h"

#include "Game/Legit/Triggerbot.hpp"

#include "Menu/Menu.hpp"

#include "eWeaponType.h"

void Triggerbot::Update()
{
    static DWORD acquireTick = 0;
    static bool wasOn = false;

    if (!g_cfg.trigger || menu->GetState())
    {
        wasOn = false;
        return;
    }

    CPed* pLocal = FindPlayerPed();

    if (!pLocal || pLocal->m_pVehicle)
    {
        wasOn = false;
        return;
    }

    CWeapon* wpn = pLocal->GetWeapon();

    if (!wpn)
    {
        wasOn = false;
        return;
    }

    eWeaponType wt = wpn->m_eWeaponType;

    if (wt < WEAPONTYPE_PISTOL || wt > WEAPONTYPE_MINIGUN)
    {
        wasOn = false;
        return;
    }

    CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
    CVector dir = cam.m_vecFront;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    if (len <= 0.001f)
    {
        wasOn = false;
        return;
    }

    dir.x /= len;
    dir.y /= len;
    dir.z /= len;

    CVector origin = cam.m_vecSource;
    CVector end{ origin.x + dir.x * 1000.0f, origin.y + dir.y * 1000.0f, origin.z + dir.z * 1000.0f };
    CColPoint colpoint{};
    CEntity* hit = nullptr;

    if (!CWorld::ProcessLineOfSight(origin, end, colpoint, hit, true, true, true, true, false, false, false, false))
    {
        wasOn = false;
        return;
    }

    if (!hit || hit == pLocal || hit->m_nType != 3)
    {
        wasOn = false;
        return;
    }

    CPed* ped = reinterpret_cast<CPed*>(hit);

    if (ped->m_fHealth <= 0.0f)
    {
        wasOn = false;
        return;
    }

    if (!wasOn)
    {
        acquireTick = GetTickCount();
        wasOn = true;
    }

    DWORD delay = (DWORD)std::clamp(g_cfg.triggerdelay, 0.0f, 500.0f);

    if (GetTickCount() - acquireTick < delay)
    {
        return;
    }

    CVector hitpos = colpoint.m_vecPoint;
    wpn->Fire(pLocal, &origin, &origin, ped, &hitpos, nullptr);
}
