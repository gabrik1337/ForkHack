#include <d3d9.h>

#include "imgui_impl_dx9.h"

#include "minhook.hpp"

#include "Utils/xorstr.h"
#include "Utils/D3D9.hpp"

#include "Gfx/Blur.hpp"

#include "Reset.hpp"

using tReset = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
tReset oReset = nullptr;

static LPVOID sResetTarget = nullptr;

HRESULT __stdcall hkReset(IDirect3DDevice9* self, D3DPRESENT_PARAMETERS* presentationParameters)
{
    Blur::OnReset();
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT result = oReset(self, presentationParameters);

    ImGui_ImplDX9_CreateDeviceObjects();

    return result;
}

void Reset::InstallHook()
{
    MH_STATUS status;

    sResetTarget = Utils::get_function_address(16);

    if (!sResetTarget)
    {
        MessageBoxA(NULL, "Failed to find device for d3d9/reset", "ForkHack", MB_OK | MB_ICONERROR);
        return;
    }

    status = MH_CreateHook(sResetTarget, &hkReset, reinterpret_cast<LPVOID*>(&oReset));
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to create hook on function d3d9/reset", "ForkHack", MB_OK | MB_ICONERROR);
    }

    status = MH_EnableHook(sResetTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to enable hook on function d3d9/reset", "ForkHack", MB_OK | MB_ICONERROR);
    }
}

void Reset::RemoveHook()
{
    MH_STATUS status;

    if (sResetTarget == nullptr)
    {
        return;
    }

    status = MH_DisableHook(sResetTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to disable hook on function d3d9/reset", "ForkHack", MB_OK | MB_ICONERROR);
    }

    status = MH_RemoveHook(sResetTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to remove hook on function d3d9/reset", "ForkHack", MB_OK | MB_ICONERROR);
    }
}
