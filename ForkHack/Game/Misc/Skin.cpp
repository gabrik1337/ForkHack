#include "Game/Features.h"

#include "Game/Misc/Skin.hpp"

void Skin::Update()
{
    int iSkin = g_cfg.playerSkinID;
    CPed* pPedSelf = FindPlayerPed();

    if (iSkin < 0)
    {
        return;
    }

    if (g_cfg.changemodel && pPedSelf)
    {
        CStreaming::RequestModel(iSkin, 0);
        CStreaming::LoadAllRequestedModels(false);
        pPedSelf->SetModelIndex((unsigned int)iSkin);
    }
}
