#pragma once

#include <windows.h>

#include <cmath>
#include <cstring>
#include <map>

#include "Utils/Sdk.hpp"
#include "Core/Config.hpp"

constexpr unsigned int REM_SHADOWS = 1u << 0;
constexpr unsigned int REM_SUN = 1u << 1;
constexpr unsigned int REM_CLOUDS = 1u << 2;
constexpr unsigned int WH_BOX = 1u << 0;
constexpr unsigned int WH_HP = 1u << 1;
constexpr unsigned int WH_ARMOR = 1u << 2;
constexpr unsigned int WH_DIST = 1u << 3;
constexpr unsigned int WH_SKELETON = 1u << 4;
constexpr unsigned int WH_SNAP = 1u << 5;
constexpr unsigned int NC_VEH = 1u << 0;
constexpr unsigned int NC_PED = 1u << 1;
constexpr unsigned int NC_OBJ = 1u << 2;

inline float VecLength(CVector vec)
{
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

inline void PatchBytes(void* addr, const unsigned char* data, size_t len)
{
    DWORD oldProt = 0;

    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProt))
    {
        return;
    }

    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProt, &oldProt);
}
