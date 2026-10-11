#include "minhook.hpp"

#include "Utils/PatternScan.hpp"
#include "Utils/SString.h"

#include "Spoofer.hpp"

#include <windows.h>
#include <wincred.h>

#include <cstring>
#include <random>
#include <string>

namespace
{
    char serial_to_spoof[33] = "";

    using tGenerateNT2Public = void** (__cdecl*)(void* a1, int a2, int* a3, int a4);
    tGenerateNT2Public pGenerateNT2Public = nullptr;

    using tGenerateNTPublic = void** (__cdecl*)(void* a1, int a2, int* a3, int a4);
    tGenerateNTPublic pGenerateNTPublic = nullptr;

    using tGenerateCheckSum = char(__cdecl*)(SString* a1);
    tGenerateCheckSum pGenerateCheckSum = nullptr;

    using tGenerateNT2Public_FunctionA = int(__fastcall*)(DWORD* a1, int a2);
    tGenerateNT2Public_FunctionA pGenerateNT2Public_FunctionA = nullptr;

    bool updated_serial = false;
    bool spoofEnabled = false;
    DWORD* safe_ecx = nullptr;
    int safe_edx = 0;

    std::string EncSerial(const std::string& input)
    {
        std::string result = input;
        result.resize(32);

        for (int i = 0; i < 32; ++i)
        {
            result[i] = ((i ^ result[i] ^ ((144 - 36 * (i & 3)) / 5) ^ 0x13 ^ (1 << (i & 7))) & 0x7F) | 0x80;
        }

        return result;
    }

    void** __cdecl Hook_GenerateNTSerial(void* a1, int a2, int* a3, int a4)
    {
        if (!pGenerateNTPublic)
            return nullptr;

        void** pData = pGenerateNTPublic(a1, a2, a3, a4);
        if (!pData || !*pData)
            return pData;

        char* szData = reinterpret_cast<char*>(*pData);

        if (spoofEnabled && updated_serial && serial_to_spoof[0] != '\0')
        {
            strncpy(szData, serial_to_spoof, 32);
            szData[32] = '\0';
        }

        return pData;
    }

    void** __cdecl Hook_GenerateNT2Serial(void* a1, int a2, int* a3, int a4)
    {
        if (!pGenerateNT2Public)
            return nullptr;

        void** pData = pGenerateNT2Public(a1, a2, a3, a4);
        if (!pData || !*pData)
            return pData;

        char* szData = reinterpret_cast<char*>(*pData);

        if (spoofEnabled && updated_serial && serial_to_spoof[0] != '\0' && pGenerateCheckSum)
        {
            std::string newEncrypted = EncSerial(serial_to_spoof);
            SString strCheckSum(newEncrypted.c_str(), 32);
            pGenerateCheckSum(&strCheckSum);
            strncpy(szData, strCheckSum.data(), 36);
            szData[36] = '\0';
        }

        return pData;
    }

    int __fastcall Hook_GenerateNT2Function(DWORD* ecx, int edx)
    {
        safe_ecx = ecx;
        safe_edx = edx;

        if (!pGenerateNT2Public_FunctionA)
            return 0;

        return pGenerateNT2Public_FunctionA(ecx, edx);
    }

    void* ptrNet = nullptr;
    std::string g_serial = "D9A8C15999B86DD5F25E8E7AD6B7C035";

    using tGenerateSerial = void(__thiscall*)(void*);
    tGenerateSerial oGenerateSerial = nullptr;
    tGenerateSerial oPreGenerateSerial = nullptr;

    std::string GetSerial_()
    {
        return g_serial;
    }

    void __fastcall hkPreGenerateSerial(void* self, void*)
    {
        ptrNet = self;
        oPreGenerateSerial(self);
    }

    void Generate()
    {
        oGenerateSerial(ptrNet);
    }

    std::string newSerial;

    using tGetDiskDriveInfo = bool(__cdecl*)(const char*, const char*, int);
    tGetDiskDriveInfo oGetDiskDriveInfo = nullptr;

    bool hkGetDiskDriveInfo(const char* diskDriveSerial, const char* diskDriveModel, int a3)
    {
        if (newSerial.empty())
        {
            newSerial = GetSerial_();
        }

        return oGetDiskDriveInfo(newSerial.data(), "S", a3);
    }

    inline std::string GenerateRandomSerial()
    {
        static constexpr char kChars[] = "0123456789ABCDEF";
        static std::mt19937 rng{ std::random_device{}() };
        static std::uniform_int_distribution<int> dist(0, 15);
        std::string s;
        s.reserve(32);
        for (int i = 0; i < 32; ++i)
            s += kChars[dist(rng)];
        return s;
    }
}

void Spoofer::ApplyCustom(const std::string& serial)
{
    if (serial.empty())
        return;

    std::string cut = serial.substr(0, 32);
    strncpy_s(serial_to_spoof, sizeof(serial_to_spoof), cut.c_str(), _TRUNCATE);
    updated_serial = true;
    spoofEnabled = true;

    LPCWSTR targetName = L"SSO_RND_Device";
    CredDeleteW(targetName, CRED_TYPE_GENERIC, 0);

    if (safe_ecx && pGenerateNT2Public_FunctionA)
    {
        Hook_GenerateNT2Function(safe_ecx, safe_edx);
    }
}

void Spoofer::ApplyRandom()
{
    serial_to_spoof[0] = '\0';
    updated_serial = false;
    spoofEnabled = false;

    std::string rnd = GenerateRandomSerial();
    g_serial = rnd;
    newSerial = rnd;

    LPCWSTR targetName = L"SSO_RND_Device";
    CredDeleteW(targetName, CRED_TYPE_GENERIC, 0);

    if (ptrNet && oGenerateSerial)
    {
        oGenerateSerial(ptrNet);
    }
}

void Spoofer::InstallHook()
{
    if (const auto& target = Utils::PatternScan("netc.dll", "E8 ? ? ? ? 8B CE E8 ? ? ? ? 8D 45 F4", true))
    {
        oGenerateSerial = (tGenerateSerial)target;
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "E8 ? ? ? ? 6A 54 E8 ? ? ? ?", true))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &hkPreGenerateSerial, reinterpret_cast<LPVOID*>(&oPreGenerateSerial));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }

    if (const auto& target = Utils::PatternScan(
        "netc.dll",
        "55 8B EC 6A ?? 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 83 EC ?? 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 ?? 64 A3 ?? ?? ?? ?? 8B 4D ?? 8D 51", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &hkGetDiskDriveInfo, reinterpret_cast<LPVOID*>(&oGetDiskDriveInfo));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C 24 ? 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 53 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? 56 57 50 8D 45 ? 64 A3 ? ? ? ? 89 8D ? ? ? ? C7 85", false))
    {
        pGenerateNT2Public_FunctionA = (tGenerateNT2Public_FunctionA)target;
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &Hook_GenerateNT2Function, reinterpret_cast<LPVOID*>(&pGenerateNT2Public_FunctionA));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? 56 57 50 8D 45 ? 64 A3 ? ? ? ? 8B 45 ? 0F 57 C0 8B 7D ? 33 F6 89 85 ? ? ? ? 0F 11 45 ? 89 85 ? ? ? ? 89 BD ? ? ? ? 89 75 ? C7 45 ? ? ? ? ? C6 45 ? ? 8B 4F ? 33 D2 ? ? 8B C1 89 75 ? 89 95 ? ? ? ? 2B C7 0F 84 ? ? ? ? 2B CF 3B D1 73 ? 0F B6 F2 B8 ? ? ? ? 8B CE 83 E1 ? 2B C1 69 C8 ? ? ? ? B8 ? ? ? ? F7 E9 8B 85 ? ? ? ? ? ? 8B CA C1 E9 ? ? ? ? ? 03 CA 33 C8 33 CE 83 E6 ? 83 F1 ? 0F BB F1 EB ? 32 C9 0F B6 F1 56 E8 ? ? ? ? 83 C4 ? 85 C0 0F 84 ? ? ? ? 8A 8E ? ? ? ? 8B 7D", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &Hook_GenerateNT2Serial, reinterpret_cast<LPVOID*>(&pGenerateNT2Public));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? 56 57 50 8D 45 ? 64 A3 ? ? ? ? 8B 45 ? 0F 57 C0 8B 7D ? 33 F6 89 85 ? ? ? ? 0F 11 45 ? 89 85 ? ? ? ? 89 BD ? ? ? ? 89 75 ? C7 45 ? ? ? ? ? C6 45 ? ? 8B 4F ? 33 D2 ? ? 8B C1 89 75 ? 89 95 ? ? ? ? 2B C7 0F 84 ? ? ? ? 2B CF 3B D1 73 ? 0F B6 F2 B8 ? ? ? ? 8B CE 83 E1 ? 2B C1 69 C8 ? ? ? ? B8 ? ? ? ? F7 E9 8B 85 ? ? ? ? ? ? 8B CA C1 E9 ? ? ? ? ? 03 CA 33 C8 33 CE 83 E6 ? 83 F1 ? 0F BB F1 EB ? 32 C9 0F B6 F1 56 E8 ? ? ? ? 83 C4 ? 85 C0 0F 84 ? ? ? ? 8A 8E ? ? ? ? 8B 75", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &Hook_GenerateNTSerial, reinterpret_cast<LPVOID*>(&pGenerateNTPublic));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 53 56 57 8B 7D ? 33 F6 32 DB", false))
    {
        pGenerateCheckSum = (tGenerateCheckSum)target;
    }
}
