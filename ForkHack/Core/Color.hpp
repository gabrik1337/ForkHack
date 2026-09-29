#pragma once

#include <cstdint>

#include "imgui.h"

class c_color
{
    uint8_t data[4]{ 255, 255, 255, 255 };

public:
    c_color();
    c_color(uint32_t color);
    c_color(int r, int g, int b, int a = 255);
    uint8_t r() const;
    uint8_t g() const;
    uint8_t b() const;
    uint8_t a() const;
    uint32_t u32() const;
    ImColor as_imcolor() const;
    ImVec4 as_imvec4() const;
    c_color new_alpha(int a) const;
    c_color increase(int v) const;
    c_color multiply(const c_color& other, float strength) const;
    double hue() const;
    double saturation() const;
    double brightness() const;
    static c_color hsb(float hue, float saturation, float brightness);
    bool operator==(const c_color& other) const;
};

class c_float_color
{
    float data[4]{ 1.0f, 1.0f, 1.0f, 1.0f };

public:
    c_float_color();
    c_float_color(int r, int g, int b, int a = 255);
    float* base();
    float* float_base();
    float& operator[](int index);
    c_color to_color(int alpha_override = -1) const;
};
