#include "minhook.hpp"

#include "Utils/xorstr.h"
#include "Utils/PatternScan.hpp"

#include "Bypass.hpp"

using tExecuteSecurityViolationKick = int(__fastcall*)(void*, void*);
tExecuteSecurityViolationKick oExecuteSecurityViolationKick;

ptrSendPacket callSendPacket = nullptr;

using tSendClientKick = int(__fastcall*)(void*, void*, char);
tSendClientKick oSendClientKick;

using tSendReport = void(__fastcall*)(void*, void*, int, void*, int, int, int);
tSendReport oSendReport;

int __fastcall HookExecuteSecurityViolationKick(void*, void*) { return 0; }
int __fastcall HookSendClientKick(void*, void*, char) { return 1; }
void __fastcall HookSendReport(void*, void*, int, void*, int, int, int) {}

bool __fastcall SendPacket(void* ECX, void* EDX, unsigned char ucPacketID, void* bitStream, int packetPriority, int packetReliability, int packetOrdering)
{
    if (ucPacketID == 91 || ucPacketID == 92 || ucPacketID == 93)
    {
        return true;
    }

    return callSendPacket(ECX, ucPacketID, bitStream, packetPriority, packetReliability, packetOrdering);
}

void Bypass::InstallHook()
{
    MessageBeep(MB_ICONASTERISK);

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 83 EC 64 A1 ? ? ? ? 33 C5 89 45 F0 56 57 50 8D 45 F4 64 A3 00 00 00 00 8B F1 50 B8 FE 14 71 4B B8 DE 2C EB 61 B8 8E 82 D9 85 B8 C2 CC B3 92 B8 3E A4 D9", false))
    {
        MH_RemoveHook(reinterpret_cast<LPVOID>(target));
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookExecuteSecurityViolationKick, reinterpret_cast<LPVOID*>(&oExecuteSecurityViolationKick));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? A1 ? ? ? ? 33 C5 89 45 ? 56 57 50 8D 45 ? 64 A3 ? ? ? ? 8B F1 50 B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? B8 ? ? ? ? 58 8B 86", false))
    {
        MH_RemoveHook(reinterpret_cast<LPVOID>(target));
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookExecuteSecurityViolationKick, reinterpret_cast<LPVOID*>(&oExecuteSecurityViolationKick));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 64 A1 ? ? ? ? 33 C5 89 45 F0 56 57 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 8B 86 ? ? ? ? 85 C0 0F 84 ? ? ? ? 83 C0 FF 89 86 ? ? ? ? 0F 85 ? ? ? ? 8D 4D 90", false))
    {
        MH_RemoveHook(reinterpret_cast<LPVOID>(target));
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookExecuteSecurityViolationKick, reinterpret_cast<LPVOID*>(&oExecuteSecurityViolationKick));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else
    {
        MessageBoxA(NULL, "Failed to find signature for gowno", "ForkHack", MB_OK | MB_ICONERROR);
    }


    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 F0 56 57 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 89 7D 84 50", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookSendClientKick, reinterpret_cast<LPVOID*>(&oSendClientKick));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else
    {
        MessageBoxA(NULL, "Failed to find signature for SendClientKick", "ForkHack", MB_OK | MB_ICONERROR);
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 F0 53 56 57 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 8B 45 18 8B 7D 08 8B 5D 0C 89 85 ? ? ? ? E8 ? ? ? ? 84 C0 75 1A FF B5 ? ? ? ?", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookSendReport, reinterpret_cast<LPVOID*>(&oSendReport));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC D8 00 00 00 A1 ? ? ? ? 33 C5 89 45 F0 53 56 57", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &HookSendReport, reinterpret_cast<LPVOID*>(&oSendReport));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else
    {
        MessageBoxA(NULL, "Failed to find signature for SendReport", "ForkHack", MB_OK | MB_ICONERROR);
    }

    if (const auto& target = Utils::PatternScan("netc.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 F0 56 57 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 89 B5 ? ? ? ? 8B 7D 0C", false))
    {
        MH_CreateHook(reinterpret_cast<LPVOID>(target), &SendPacket, reinterpret_cast<LPVOID*>(&callSendPacket));
        MH_EnableHook(reinterpret_cast<LPVOID>(target));
    }
    else
    {
        MessageBoxA(NULL, "Failed to find signature for SendPacket", "ForkHack", MB_OK | MB_ICONERROR);
    }
}
