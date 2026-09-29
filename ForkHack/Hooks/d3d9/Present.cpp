#include <d3d9.h>

#include <algorithm>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include "minhook.hpp"

#include "Utils/xorstr.h"
#include "Utils/D3D9.hpp"

#include "Gfx/Blur.hpp"
#include "Gfx/Fonts.hpp"
#include "Menu/Menu.hpp"
#include "Core/Config.hpp"
#include "Game/Features.h"

#include "Present.hpp"

#include "../core/ForceCursorVisible.hpp"

using tPresent = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
tPresent oPresent = nullptr;

static LPVOID sPresentTarget = nullptr;

static WNDPROC oWndProc = nullptr;
static bool imgui_initialized = false;
static HWND hGameWindow = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);


LRESULT WINAPI WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    const bool is_open = (menu && menu->GetState());

    if (is_open && ImGui::GetCurrentContext())
    {
        ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam);
    }

    if (is_open)
    {
        if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
            || (message >= WM_KEYFIRST && message <= WM_KEYLAST)
            || message == WM_INPUT)
        {
            return 1;
        }
    }

    return oWndProc
        ? CallWindowProcW(oWndProc, window, message, wParam, lParam)
        : DefWindowProcW(window, message, wParam, lParam);
}

static void InitImGui(IDirect3DDevice9* device)
{
    D3DDEVICE_CREATION_PARAMETERS deviceParameters;
    device->GetCreationParameters(&deviceParameters);

    oWndProc = (WNDPROC)SetWindowLongPtrW(deviceParameters.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)WndProcHandler);
    hGameWindow = deviceParameters.hFocusWindow;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    ImGui::StyleColorsDark();
    Fonts::InitStyle();
    ImGui::GetStyle().AntiAliasedLines = false;
    ImGui::GetStyle().AntiAliasedFill = false;
    g_fonts.Init();

    ImGui_ImplWin32_Init(deviceParameters.hFocusWindow);
    ImGui_ImplDX9_Init(device);
    Blur::SetDevice(device);
}

HRESULT __stdcall hkPresent(IDirect3DDevice9* self, const RECT* sourceRect, const RECT* destRect, HWND destWindowOverride, const RGNDATA* dirtyRegion)
{
    if (!imgui_initialized)
    {
        InitImGui(self);
        imgui_initialized = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::GetIO().FontGlobalScale = g_cfg.ui_scale / 100.0f;

    KeyBinds::Update();
    World::Update();
    RapidFire::Update();
    InfAmmo::Update();
    GameSpeed::Update();
    FastRot::Update();
    VehicleFlags::Update();
    NoBikeFall::Update();
    Aspect::Update();
    Fov::Update();
    AutoEngine::Update();
    AutoUnlock::Update();
    FastCrosshair::Update();
    NoRecoil::Update();
    NoSpread::Update();
    Triggerbot::Update();
    NoCamCol::Update();
    CamHack::Update();
    CamHack::Update();

    Esp::Update();

    GodMode::Update();
    NoFall::Update();
    FastRun::Update();

    if (g_cfg.speedhack && !g_cfg.airbreake)
    {
        static DWORD lastTick = 0;
        DWORD now = GetTickCount();

        if (now - lastTick >= 50)
        {
            lastTick = now;

            CPed* pPedSelf = FindPlayerPed();

            if (pPedSelf)
            {
                CVehicle* veh = pPedSelf->m_pVehicle;

                if (veh && veh->m_pDriver == pPedSelf)
                {
                    CVector& vel = veh->m_vecMoveSpeed;
                    float direction = VecLength(vel);

                    if (direction > 0.01f)
                    {
                        float power = std::clamp(g_cfg.MaxSpd, 0.0f, 30.0f);
                        float boost = power / 180.0f;
                        vel.x += (vel.x / direction) * boost;
                        vel.y += (vel.y / direction) * boost;
                        vel.z += (vel.z / direction) * boost;
                    }
                }
            }
        }
    }
    Blur::NewFrame();

    if (GetAsyncKeyState(VK_DELETE) & 1)
    {
        menu->ToggleState();

        if (Cself)
        {
            if (menu->GetState())
            {
                callForceCursorVisible(Cself, true, true);
            }
            else
            {
                callForceCursorVisible(Cself, false, false);
            }
        }
    }

    menu->Draw();
    menu->DrawBinds();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    const bool is_open = menu->GetState();

    if (is_open && hGameWindow)
    {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(hGameWindow, &p);
        ImGui::GetForegroundDrawList()->AddTriangleFilled(ImVec2(p.x - 10, p.y), ImVec2(p.x, p.y + 10), ImVec2(p.x + 10, p.y), g_cfg.accent.to_color().as_imcolor());
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oPresent(self, sourceRect, destRect, destWindowOverride, dirtyRegion);
}

void Present::InstallHook()
{
    MH_STATUS status;

    sPresentTarget = Utils::get_function_address(17);

    if (!sPresentTarget)
    {
        MessageBoxA(NULL, "Failed to find device for d3d9/present", "ForkHack", MB_OK | MB_ICONERROR);
        return;
    }

    status = MH_CreateHook(sPresentTarget, &hkPresent, reinterpret_cast<LPVOID*>(&oPresent));
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to create hook on function d3d9/present", "ForkHack", MB_OK | MB_ICONERROR);
    }

    status = MH_EnableHook(sPresentTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to enable hook on function d3d9/present", "ForkHack", MB_OK | MB_ICONERROR);
    }
}

void Present::RemoveHook()
{
    MH_STATUS status;

    if (sPresentTarget == nullptr)
    {
        return;
    }

    status = MH_DisableHook(sPresentTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to disable hook on function d3d9/present", "ForkHack", MB_OK | MB_ICONERROR);
    }

    status = MH_RemoveHook(sPresentTarget);
    if (status != MH_OK)
    {
        MessageBoxA(NULL, "Failed to remove hook on function d3d9/present", "ForkHack", MB_OK | MB_ICONERROR);
    }
}
