#pragma once

#include <d3d9.h>

#include "imgui.h"
#include "imgui_internal.h"

namespace Blur
{
    void SetDevice(IDirect3DDevice9* device);
    IDirect3DDevice9* GetDevice();
    void ClearTextures();
    void OnReset();
    void NewFrame();
    void Create(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col = ImColor(255, 255, 255, 255), float rounding = 0.0f, int round_flags = 15);
}
