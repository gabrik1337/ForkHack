#include "minhook.hpp"

#include <string>
#include <wchar.h>

#include "Utils/xorstr.h"

#include "../netc/Bypass.hpp"
#include "../core/ForceCursorVisible.hpp"

#include "LdrDll.hpp"

ptrLdrLoadDll callLdrLoadDll = nullptr;

namespace
{
    bool netc_once = false;
    LPVOID sLdrTarget = nullptr;

    bool EndsWithIC(const std::wstring& s, const wchar_t* suffix)
    {
        size_t n = wcslen(suffix);

        if (s.size() < n)
        {
            return false;
        }

        return _wcsnicmp(s.c_str() + s.size() - n, suffix, n) == 0;
    }

    NTSTATUS NTAPI hkLdrLoadDll(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle)
    {
        NTSTATUS sts = callLdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);

        if (ModuleFileName && ModuleFileName->Buffer)
        {
            std::wstring name(ModuleFileName->Buffer, ModuleFileName->Length / sizeof(WCHAR));

            if (!netc_once && EndsWithIC(name, L"netc.dll"))
            {
                netc_once = true;
                Bypass::InstallHook();
            }

            if (EndsWithIC(name, L"core.dll"))
            {
                ForceCursor::InstallHook();
            }
        }

        return sts;
    }
}

void LdrDll::InstallHook()
{
    callLdrLoadDll = (ptrLdrLoadDll)GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll");

    if (callLdrLoadDll != nullptr)
    {
        sLdrTarget = reinterpret_cast<LPVOID>(callLdrLoadDll);
        MH_CreateHook(sLdrTarget, reinterpret_cast<LPVOID>(&hkLdrLoadDll), reinterpret_cast<LPVOID*>(&callLdrLoadDll));
        MH_EnableHook(sLdrTarget);
    }
}

void LdrDll::RemoveHook()
{
    if (sLdrTarget == nullptr)
    {
        return;
    }

    MH_DisableHook(sLdrTarget);
    MH_RemoveHook(sLdrTarget);
    sLdrTarget = nullptr;
}
