#include "Core/Color.hpp"

#include <algorithm>
#include <cmath>

c_color::c_color()
{
    data[0] = 255;
    data[1] = 255;
    data[2] = 255;
    data[3] = 255;
}

c_color::c_color(uint32_t color)
{
    data[0] = (uint8_t)(color & 255);
    data[1] = (uint8_t)((color >> 8) & 255);
    data[2] = (uint8_t)((color >> 16) & 255);
    data[3] = (uint8_t)((color >> 24) & 255);
}

c_color::c_color(int r, int g, int b, int a)
{
    data[0] = (uint8_t)std::clamp(r, 0, 255);
    data[1] = (uint8_t)std::clamp(g, 0, 255);
    data[2] = (uint8_t)std::clamp(b, 0, 255);
    data[3] = (uint8_t)std::clamp(a, 0, 255);
}

uint8_t c_color::r() const
{
    return data[0];
}

uint8_t c_color::g() const
{
    return data[1];
}

uint8_t c_color::b() const
{
    return data[2];
}

uint8_t c_color::a() const
{
    return data[3];
}

uint32_t c_color::u32() const
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

ImColor c_color::as_imcolor() const
{
    return ImColor(data[0], data[1], data[2], data[3]);
}

ImVec4 c_color::as_imvec4() const
{
    return ImVec4(data[0] / 255.0f, data[1] / 255.0f, data[2] / 255.0f, data[3] / 255.0f);
}

c_color c_color::new_alpha(int a) const
{
    return c_color(data[0], data[1], data[2], std::clamp(a, 0, 255));
}

c_color c_color::increase(int v) const
{
    return c_color(data[0] + v, data[1] + v, data[2] + v, data[3]);
}

c_color c_color::multiply(const c_color& other, float strength) const
{
    if (data[0] == other.data[0] && data[1] == other.data[1] && data[2] == other.data[2])
    {
        return *this;
    }

    return c_color(
        (int)(data[0] + (other.data[0] - data[0]) * strength),
        (int)(data[1] + (other.data[1] - data[1]) * strength),
        (int)(data[2] + (other.data[2] - data[2]) * strength),
        data[3]);
}

double c_color::hue() const
{
    double r = data[0] / 255.0;
    double g = data[1] / 255.0;
    double b = data[2] / 255.0;
    double mx = std::max<double>(r, std::max<double>(g, b));
    double mn = std::min<double>(r, std::min<double>(g, b));

    if (mx == mn)
    {
        return 0.0;
    }

    double delta = mx - mn;
    double hue = 0.0;

    if (mx == r)
    {
        hue = (g - b) / delta;
    }
    else if (mx == g)
    {
        hue = 2.0 + (b - r) / delta;
    }
    else
    {
        hue = 4.0 + (r - g) / delta;
    }

    hue *= 60.0;

    if (hue < 0.0)
    {
        hue += 360.0;
    }

    return hue / 360.0;
}

double c_color::saturation() const
{
    double r = data[0] / 255.0;
    double g = data[1] / 255.0;
    double b = data[2] / 255.0;
    double mx = std::max<double>(r, std::max<double>(g, b));
    double mn = std::min<double>(r, std::min<double>(g, b));
    double delta = mx - mn;

    if (mx == 0.0)
    {
        return delta;
    }

    return delta / mx;
}

double c_color::brightness() const
{
    double r = data[0] / 255.0;
    double g = data[1] / 255.0;
    double b = data[2] / 255.0;

    return std::max<double>(r, std::max<double>(g, b));
}

c_color c_color::hsb(float hue, float saturation, float brightness)
{
    hue = std::clamp<float>(hue, 0.0f, 1.0f);
    saturation = std::clamp<float>(saturation, 0.0f, 1.0f);
    brightness = std::clamp<float>(brightness, 0.0f, 1.0f);

    float h = (hue == 1.0f) ? 0.0f : (hue * 6.0f);
    float f = h - (float)(int)h;
    float p = brightness * (1.0f - saturation);
    float q = brightness * (1.0f - saturation * f);
    float t = brightness * (1.0f - (saturation * (1.0f - f)));

    if (h < 1.0f)
    {
        return c_color((int)(brightness * 255), (int)(t * 255), (int)(p * 255));
    }
    else if (h < 2.0f)
    {
        return c_color((int)(q * 255), (int)(brightness * 255), (int)(p * 255));
    }
    else if (h < 3.0f)
    {
        return c_color((int)(p * 255), (int)(brightness * 255), (int)(t * 255));
    }
    else if (h < 4.0f)
    {
        return c_color((int)(p * 255), (int)(q * 255), (int)(brightness * 255));
    }
    else if (h < 5.0f)
    {
        return c_color((int)(t * 255), (int)(p * 255), (int)(brightness * 255));
    }

    return c_color((int)(brightness * 255), (int)(p * 255), (int)(q * 255));
}

bool c_color::operator==(const c_color& other) const
{
    return data[0] == other.data[0] && data[1] == other.data[1] && data[2] == other.data[2] && data[3] == other.data[3];
}

c_float_color::c_float_color()
{
}

c_float_color::c_float_color(int r, int g, int b, int a)
{
    data[0] = std::clamp(r, 0, 255) / 255.0f;
    data[1] = std::clamp(g, 0, 255) / 255.0f;
    data[2] = std::clamp(b, 0, 255) / 255.0f;
    data[3] = std::clamp(a, 0, 255) / 255.0f;
}

float* c_float_color::base()
{
    return data;
}

float* c_float_color::float_base()
{
    return data;
}

float& c_float_color::operator[](int index)
{
    return data[index];
}

c_color c_float_color::to_color(int alpha_override) const
{
    int alpha = alpha_override < 0 ? (int)(data[3] * 255.0f) : alpha_override;

    return c_color((int)(data[0] * 255.0f), (int)(data[1] * 255.0f), (int)(data[2] * 255.0f), alpha);
}
