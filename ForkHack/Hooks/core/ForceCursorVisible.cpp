#include "minhook.hpp"

#include "Menu/Menu.hpp"

#include "Utils/PatternScan.hpp"
#include "Utils/xorstr.h"

#include "ForceCursorVisible.hpp"
void* Cself = nullptr;
ForceCursorVisibleFn callForceCursorVisible = nullptr;
void __fastcall ForceCursorVisible(void* ECX, void* EDX, bool bVisible, bool bToggleControls)
{
    Cself = ECX;
    callForceCursorVisible(ECX, bVisible, bToggleControls);
}

void ForceCursor::InstallHook()
{
    if (callForceCursorVisible == nullptr)
    {
        if (const auto& target = Utils::PatternScan("core.dll", "55 8B EC 8A 45 0C FF 75 08 88 81 00 00 00 00 8B 49 0C E8 00 00 00 00 5D C2 08 00", false))
        {
            callForceCursorVisible = (ForceCursorVisibleFn)target;
            MH_CreateHook(reinterpret_cast<LPVOID>(target), reinterpret_cast<LPVOID>(&ForceCursorVisible), reinterpret_cast<LPVOID*>(&callForceCursorVisible));
            MH_EnableHook(reinterpret_cast<LPVOID>(target));
        }
        else if (const auto& target2 = Utils::PatternScan("core.dll", "55 8B EC 8A 45 0C FF", false))
        {
            callForceCursorVisible = (ForceCursorVisibleFn)target2;
            MH_CreateHook(reinterpret_cast<LPVOID>(target2), reinterpret_cast<LPVOID>(&ForceCursorVisible), reinterpret_cast<LPVOID*>(&callForceCursorVisible));
            MH_EnableHook(reinterpret_cast<LPVOID>(target2));
        }
    }
}