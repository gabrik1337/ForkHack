#pragma once

#include <string>

class Spoofer
{
public:
    static void InstallHook();
    static void ApplyCustom(const std::string& serial);
    static void ApplyRandom();
};
