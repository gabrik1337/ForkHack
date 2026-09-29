#include "minhook.hpp"

#include "../../thirdparty/virtualiser/VirtualizerSDK.h"

#include "Utils/xorstr.h"

#include "Collision/Collision.hpp"

#include "LdrDll/LdrDll.hpp"

#include "d3d9/Present.hpp"
#include "d3d9/Reset.hpp"

#include "Hooks.hpp"

void Hooks::InstallHooks()
{
    MH_STATUS status;

    status = MH_Initialize();
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to initialize MinHook", "ForkHack", MB_OK | MB_ICONERROR);
        return;
    }
    VIRTUALIZER_START;
    LdrDll::InstallHook();

    Collision::InstallHook();

    Present::InstallHook();
    Reset::InstallHook();
    VIRTUALIZER_END;
}

void Hooks::RemoveHooks()
{
    MH_STATUS status;
    Present::RemoveHook();
    Reset::RemoveHook();

    status = MH_Uninitialize();
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to uninitialize MinHook", "ForkHack", MB_OK | MB_ICONERROR);
    }
}
