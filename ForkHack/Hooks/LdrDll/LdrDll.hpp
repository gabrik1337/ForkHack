#pragma once

#include <winternl.h>

using ptrLdrLoadDll = NTSTATUS(NTAPI*)(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);

extern ptrLdrLoadDll callLdrLoadDll;

class LdrDll
{
public:
    static void InstallHook();
    static void RemoveHook();
};
