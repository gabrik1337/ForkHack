#pragma once

#include <d3d9.h>

class Texture
{
public:
    static bool FromMemory(IDirect3DDevice9* device, const unsigned char* data, unsigned int size, LPDIRECT3DTEXTURE9* out);
};
