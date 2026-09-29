#pragma once

#include <Psapi.h>
#include <vector>

namespace Utils
{
    static DWORD PatternScan(const char* module, const char* signature, bool isJump)
    {
        static auto PatternToByte = [](const char* pattern) {
            auto bytes = std::vector<char>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current)
            {
                if (*current == '?')
                {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back('\?');
                }
                else
                {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        MODULEINFO moduleInfo = {0};

        if (HMODULE hModule = GetModuleHandleA(module))
        {
            if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO)))
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }

        DWORD base = (DWORD)moduleInfo.lpBaseOfDll;
        DWORD sizeOfImage = (DWORD)moduleInfo.SizeOfImage;
        auto  patternBytes = PatternToByte(signature);

        DWORD patternLength = patternBytes.size();
        auto  data = patternBytes.data();

        for (DWORD i = 0; i < sizeOfImage - patternLength; i++)
        {
            bool found = true;
            for (DWORD j = 0; j < patternLength; j++)
            {
                char a = '\?';
                char b = *(char*)(base + i + j);
                found &= data[j] == a || data[j] == b;
            }
            if (found)
            {
                if (isJump)
                {
                    const auto& offset = 0xFFFFFFFF - *reinterpret_cast<DWORD*>(base + i + 1) - 0x4;
                    return base + i - offset;
                }
                else
                {
                    return base + i;
                }
            }
        }
        return NULL;
    }
} // namespace Utils
