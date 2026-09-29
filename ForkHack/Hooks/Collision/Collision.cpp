// cred: who?
#include "Collision.hpp"

#include "Core/Config.hpp"

#include <windows.h>

#include <cstdint>
#include <cstring>

namespace
{
    constexpr DWORD HOOKPOS_PlayerCollision = 0x0054BCEE;

    BYTE col_prologue[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    bool col_installed = false;

    typedef struct _VECTOR
    {
#pragma pack(1)
        float X, Y, Z;
    } VECTOR, *PVECTOR;

    typedef struct _MATRIX4X4
    {
#pragma pack(1)
        VECTOR right;
        DWORD flags;
        VECTOR up;
        float pad_u;
        VECTOR at;
        float pad_a;
        VECTOR pos;
        float pad_p;
    } MATRIX4X4, *PMATRIX4X4;

    struct object_base
    {
#pragma pack(1)
        void* vtbl;
        float coords[3];
        union
        {
            float m_heading;
            void* m_CMatrixPre;
            float* preMatrix;
            MATRIX4X4* preMatrixStruct;
        };
        union
        {
            void* m_CMatrix;
            float* matrix;
            MATRIX4X4* matrixStruct;
        };

        void* m_pRwObject;

        unsigned long bUsesCollision : 1;
        unsigned long bCollisionProcessed : 1;
        unsigned long bIsStatic : 1;
        unsigned long bHasContacted : 1;
        unsigned long bIsStuck : 1;
        unsigned long bIsInSafePosition : 1;
        unsigned long bWasPostponed : 1;
        unsigned long bIsVisible : 1;
        unsigned long bIsBIGBuilding : 1;
        unsigned long bRenderDamaged : 1;
        unsigned long bStreamingDontDelete : 1;
        unsigned long bRemoveFromWorld : 1;
        unsigned long bHasHitWall : 1;
        unsigned long bImBeingRendered : 1;
        unsigned long bDrawLast : 1;
        unsigned long bDistanceFade : 1;
        unsigned long bDontCastShadowsOn : 1;
        unsigned long bOffscreen : 1;
        unsigned long bIsStaticWaitingForCollision : 1;
        unsigned long bDontStream : 1;
        unsigned long bUnderwater : 1;
        unsigned long bHasPreRenderEffects : 1;
        unsigned long bIsTempBuilding : 1;
        unsigned long bDontUpdateHierarchy : 1;
        unsigned long bHasRoadsignText : 1;
        unsigned long bDisplayedSuperLowLOD : 1;
        unsigned long bIsProcObject : 1;
        unsigned long bBackfaceCulled : 1;
        unsigned long bLightObject : 1;
        unsigned long bUnimportantStream : 1;
        unsigned long bTunnel : 1;
        unsigned long bTunnelTransition : 1;

        uint8_t wSeedColFlags;
        uint8_t wSeedVisibleFlags;
        uint16_t model_alt_id;
        uint8_t __unknown_36[4];

        uint32_t* m_pLastRenderedLink;
        uint16_t timer;
        uint8_t m_iplIndex;
        uint8_t interior_id;
        uint8_t __unknown_48[6];

        uint8_t nType : 3;
        uint8_t nStatus : 5;

        uint8_t __unknown_56[8];
        uint8_t quantumPhysics;
        uint8_t nImmunities;
        uint8_t __unknown_66;
    };

    bool CollisionCheck(object_base* obj1, object_base* obj2)
    {
        return (obj2->nType == 3 || obj2->nType == 2) &&
            ((obj1->nType == 2 && (g_cfg.nocol_flags & 1)) ||
                (obj1->nType == 3 && (g_cfg.nocol_flags & 2)) ||
                (obj1->nType == 4 && (g_cfg.nocol_flags & 4)));
    }

    void __declspec(naked) HOOK_PlayerCollision()
    {
        static object_base* _obj1, * _obj2;
        static DWORD RETURN_ovrwr = 0x54CEFC, RETURN_process = 0x0054BCF4, RETURN_noProcessing = 0x54CF8D;
        __asm
        {
            jz hk_PlCol_process
            jmp RETURN_ovrwr
        }
    hk_PlCol_process:
        __asm
        {
            pushad
            mov _obj2, esi
            mov _obj1, edi
        }
        if (!g_cfg.nocol) goto hk_PlCol_processCol;
        if (_obj1 == nullptr || _obj2 == nullptr) goto hk_PlCol_noCol;
        if (CollisionCheck(_obj1, _obj2)) goto hk_PlCol_noCol;
    hk_PlCol_processCol:
        __asm popad
        __asm jmp RETURN_process
    hk_PlCol_noCol:
        __asm popad
        __asm jmp RETURN_noProcessing
    }
}

void Collision::InstallHook()
{
    BYTE* p = reinterpret_cast<BYTE*>(HOOKPOS_PlayerCollision);

    if (*p == 0xE9 || *p != 0x0F)
    {
        return;
    }

    memcpy(col_prologue, p, sizeof(col_prologue));

    DWORD oldProt = 0;
    VirtualProtect(p, sizeof(col_prologue), PAGE_EXECUTE_READWRITE, &oldProt);
    p[0] = 0xE9;
    *reinterpret_cast<DWORD*>(p + 1) = reinterpret_cast<DWORD>(&HOOK_PlayerCollision) - HOOKPOS_PlayerCollision - 5;
    p[5] = 0x90;
    VirtualProtect(p, sizeof(col_prologue), oldProt, &oldProt);

    col_installed = true;
}

void Collision::RemoveHook()
{
    if (!col_installed)
    {
        return;
    }

    BYTE* p = reinterpret_cast<BYTE*>(HOOKPOS_PlayerCollision);

    DWORD oldProt = 0;
    VirtualProtect(p, sizeof(col_prologue), PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(p, col_prologue, sizeof(col_prologue));
    VirtualProtect(p, sizeof(col_prologue), oldProt, &oldProt);

    col_installed = false;
}
