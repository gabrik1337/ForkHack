#pragma once

using ptrSendPacket = bool(__thiscall*)(void* ECX, unsigned char ucPacketID, void* bitStream, int packetPriority, int packetReliability, int packetOrdering);

extern ptrSendPacket callSendPacket;

class Bypass
{
public:
    static void InstallHook();
};
