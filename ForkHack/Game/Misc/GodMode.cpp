#include "Game/Features.h"

#include "Game/Misc/GodMode.hpp"

void GodMode::Update()
{
    CPed* ped = FindPlayerPed(-1);

    if (!ped)
    {
        return;
    }

    CVehicle* veh = FindPlayerVehicle(0, false);

    if (!g_cfg.godmode)
    {
        ped->bBulletProof = false;
        ped->bFireProof = false;
        ped->bCollisionProof = false;
        ped->bMeleeProof = false;
        ped->bExplosionProof = false;
        ped->bInvulnerable = false;

        if (veh)
        {
            veh->bBulletProof = false;
            veh->bFireProof = false;
            veh->bCollisionProof = false;
            veh->bMeleeProof = false;
            veh->bExplosionProof = false;
            veh->bInvulnerable = false;
        }

        return;
    }

    ped->bBulletProof = true;
    ped->bFireProof = true;
    ped->bCollisionProof = true;
    ped->bMeleeProof = true;
    ped->bExplosionProof = true;
    ped->bInvulnerable = true;

    if (veh)
    {
        veh->bBulletProof = true;
        veh->bFireProof = true;
        veh->bCollisionProof = true;
        veh->bMeleeProof = true;
        veh->bExplosionProof = true;
        veh->bInvulnerable = true;
    }
}
