#pragma once

#include <cstdint>
#include <cstring>
#include <windows.h>

#include "Utils/xorstr.h"

namespace Utils
{
    inline std::uintptr_t find_device(std::uint32_t Len)
    {
        static std::uintptr_t base = [Len]()
        {
            char sysdir[MAX_PATH];
            GetSystemDirectoryA(sysdir, MAX_PATH);
            strcat_s(sysdir, "\\d3d9.dll");

            std::uintptr_t dll = reinterpret_cast<std::uintptr_t>(LoadLibraryA(sysdir));

            if (!dll)
            {
                return std::uintptr_t(0);
            }

            std::uintptr_t end = dll + Len;
            while (dll < end)
            {
                if (*reinterpret_cast<std::uint16_t*>(dll + 0x00) == 0x06C7 &&
                    *reinterpret_cast<std::uint16_t*>(dll + 0x06) == 0x8689 &&
                    *reinterpret_cast<std::uint16_t*>(dll + 0x0C) == 0x8689)
                {
                    dll += 2;
                    break;
                }
                dll++;
            }

            if (dll >= end)
            {
                return std::uintptr_t(0);
            }

            return dll;
        }();
        return base;
    }

    inline void* get_function_address(int idx)
    {
        std::uintptr_t device = find_device(0x128000);
        if (!device)
        {
            return nullptr;
        }

        void** vtable = *reinterpret_cast<void***>(device);
        void* func = vtable[idx];

        return func;
    }
}
