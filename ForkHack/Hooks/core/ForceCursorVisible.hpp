#pragma once
using ForceCursorVisibleFn = void(__thiscall*)(void*, bool, bool);

extern void* Cself;
extern ForceCursorVisibleFn callForceCursorVisible;
class ForceCursor
{
public:
    static void InstallHook();
    static void RemoveHook();
};
