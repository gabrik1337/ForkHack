#include "Gfx/Texture.hpp"

#include <wincodec.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

bool Texture::FromMemory(IDirect3DDevice9* device, const unsigned char* data, unsigned int size, LPDIRECT3DTEXTURE9* out)
{
    if (!device || !data || size == 0 || !out)
    {
        return false;
    }

    *out = nullptr;

    ComPtr<IWICImagingFactory> factory;

    if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf())) != S_OK)
    {
        return false;
    }

    ComPtr<IWICStream> stream;

    if (factory->CreateStream(stream.GetAddressOf()) != S_OK)
    {
        return false;
    }

    if (stream->InitializeFromMemory((BYTE*)data, size) != S_OK)
    {
        return false;
    }

    ComPtr<IWICBitmapDecoder> decoder;

    if (factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()) != S_OK)
    {
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;

    if (decoder->GetFrame(0, frame.GetAddressOf()) != S_OK)
    {
        return false;
    }

    ComPtr<IWICFormatConverter> converter;

    if (factory->CreateFormatConverter(converter.GetAddressOf()) != S_OK)
    {
        return false;
    }

    if (converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom) != S_OK)
    {
        return false;
    }

    UINT width = 0;
    UINT height = 0;

    if (converter->GetSize(&width, &height) != S_OK || width == 0 || height == 0)
    {
        return false;
    }

    LPDIRECT3DTEXTURE9 texture = nullptr;

    if (device->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, nullptr) != D3D_OK)
    {
        return false;
    }

    D3DLOCKED_RECT locked{};

    if (texture->LockRect(0, &locked, nullptr, 0) != D3D_OK)
    {
        texture->Release();

        return false;
    }

    if (converter->CopyPixels(nullptr, locked.Pitch, locked.Pitch * height, (BYTE*)locked.pBits) != S_OK)
    {
        texture->UnlockRect(0);
        texture->Release();

        return false;
    }

    texture->UnlockRect(0);
    *out = texture;

    return true;
}
