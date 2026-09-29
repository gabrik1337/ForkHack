#include "Gfx/Blur.hpp"

#include <wrl/client.h>

#include "shader_blur_x.h"
#include "shader_blur_y.h"

using Microsoft::WRL::ComPtr;

static int backbufferWidth = 0;
static int backbufferHeight = 0;
static IDirect3DDevice9* device = nullptr;

static IDirect3DTexture9* create_texture(int width, int height)
{
    IDirect3DTexture9* texture = nullptr;

    if (width <= 0 || height <= 0)
    {
        return nullptr;
    }

    if (device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr) != D3D_OK)
    {
        return nullptr;
    }

    return texture;
}

static void copy_back_buffer_to_texture(IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering)
{
    ComPtr<IDirect3DSurface9> backBuffer;

    if (device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, backBuffer.GetAddressOf()) == D3D_OK)
    {
        ComPtr<IDirect3DSurface9> surface;

        if (texture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
        {
            device->StretchRect(backBuffer.Get(), nullptr, surface.Get(), nullptr, filtering);
        }
    }
}

static void set_render_target(IDirect3DTexture9* rtTexture)
{
    ComPtr<IDirect3DSurface9> surface;

    if (rtTexture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
    {
        device->SetRenderTarget(0, surface.Get());
    }
}

class c_shader_program
{
public:
    void use(float uniform, int location) const
    {
        device->SetPixelShader(pixel_shader.Get());
        const float params[4] = { uniform };
        device->SetPixelShaderConstantF(location, params, 1);
    }

    void init(const BYTE* pixelShaderSrc)
    {
        if (initialized)
        {
            return;
        }

        initialized = true;
        device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), pixel_shader.GetAddressOf());
    }

private:
    ComPtr<IDirect3DPixelShader9> pixel_shader{};
    bool initialized = false;
};

class c_blur_effect
{
public:
    static void draw(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, int round_flags)
    {
        instance().draw_impl(drawList, min, max, col, rounding, round_flags);
    }

    static void clear_textures()
    {
        if (instance().blur_texture1)
        {
            instance().blur_texture1->Release();
            instance().blur_texture1 = nullptr;
        }

        if (instance().blur_texture2)
        {
            instance().blur_texture2->Release();
            instance().blur_texture2 = nullptr;
        }
    }

    static void create_textures()
    {
        if (!instance().blur_texture1)
        {
            instance().blur_texture1 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
        }

        if (!instance().blur_texture2)
        {
            instance().blur_texture2 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
        }
    }

    static void create_shaders()
    {
        instance().blur_shaderx.init(blur_x);
        instance().blur_shadery.init(blur_y);
    }

private:
    D3DMATRIX backup_matrix{};
    IDirect3DSurface9* rt_backup = nullptr;
    IDirect3DTexture9* blur_texture1 = nullptr;
    IDirect3DTexture9* blur_texture2 = nullptr;
    c_shader_program blur_shaderx{};
    c_shader_program blur_shadery{};
    static constexpr auto blur_down_sample = 9;

    static c_blur_effect& instance()
    {
        static c_blur_effect blurEffect;

        return blurEffect;
    }

    static void begin(const ImDrawList*, const ImDrawCmd*)
    {
        instance().begin_impl();
    }

    static void first_pass(const ImDrawList*, const ImDrawCmd*)
    {
        instance().first_pass_impl();
    }

    static void second_pass(const ImDrawList*, const ImDrawCmd*)
    {
        instance().second_pass_impl();
    }

    static void end(const ImDrawList*, const ImDrawCmd*)
    {
        instance().end_impl();
    }

    void begin_impl()
    {
        device->GetRenderTarget(0, &rt_backup);
        copy_back_buffer_to_texture(blur_texture1, D3DTEXF_LINEAR);
        device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        device->GetVertexShaderConstantF(0, &backup_matrix.m[0][0], 4);

        const D3DMATRIX projection{ { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (backbufferWidth / blur_down_sample), 1.0f / (backbufferHeight / blur_down_sample), 0.0f, 1.0f } } };
        device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
    }

    void first_pass_impl()
    {
        blur_shaderx.use(1.0f / (backbufferWidth / blur_down_sample), 0);
        set_render_target(blur_texture2);
    }

    void second_pass_impl()
    {
        blur_shadery.use(1.0f / (backbufferHeight / blur_down_sample), 0);
        set_render_target(blur_texture1);
    }

    void end_impl()
    {
        device->SetRenderTarget(0, rt_backup);
        rt_backup->Release();
        device->SetPixelShader(nullptr);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    }

    void draw_impl(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, int round_flags)
    {
        if (backbufferWidth <= 0 || backbufferHeight <= 0)
        {
            return;
        }

        create_textures();
        create_shaders();

        if (!blur_texture1 || !blur_texture2)
        {
            return;
        }

        drawList->AddCallback(&begin, nullptr);

        for (int i = 0; i < 8; ++i)
        {
            drawList->AddCallback(&first_pass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blur_texture1), { -1.0f, -1.0f }, { 1.0f, 1.0f });
            drawList->AddCallback(&second_pass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blur_texture2), { -1.0f, -1.0f }, { 1.0f, 1.0f });
        }

        drawList->AddCallback(&end, nullptr);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        drawList->AddImageRounded(reinterpret_cast<ImTextureID>(blur_texture1), min, max, { min.x / backbufferWidth, min.y / backbufferHeight }, { max.x / backbufferWidth, max.y / backbufferHeight },
            ImGui::GetColorU32(col.Value), rounding, round_flags);
    }
};

namespace Blur
{
    void SetDevice(IDirect3DDevice9* target)
    {
        device = target;
    }

    IDirect3DDevice9* GetDevice()
    {
        return device;
    }

    void ClearTextures()
    {
        c_blur_effect::clear_textures();
    }

    void OnReset()
    {
        c_blur_effect::clear_textures();
    }

    void NewFrame()
    {
        const int width = (int)ImGui::GetIO().DisplaySize.x;
        const int height = (int)ImGui::GetIO().DisplaySize.y;

        if (backbufferWidth != width || backbufferHeight != height)
        {
            c_blur_effect::clear_textures();
            backbufferWidth = width;
            backbufferHeight = height;
        }
    }

    void Create(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, int round_flags)
    {
        c_blur_effect::draw(drawList, min, max, col, rounding, round_flags);
    }
}
