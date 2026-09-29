#include "Game/Features.h"

#include "Game/Visuals/Esp.hpp"

#include "Menu/Menu.hpp"

#include "imgui.h"
#include "CColModel.h"
#include "CPools.h"
#include "CSprite.h"
#include "ePedBones.h"

#include <cmath>
#include <cstdio>

void Esp::Update()
{
    if (!g_cfg.wh)
    {
        return;
    }

    CPed* pLocal = FindPlayerPed();

    if (!pLocal || !CPools::ms_pPedPool)
    {
        return;
    }

    const CVector localPos = pLocal->GetPosition();
    const float s = menu->GetScale();
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    const ImU32 boxCol = g_cfg.whcol.to_color().as_imcolor();
    const ImU32 backCol = IM_COL32(0, 0, 0, 180);
    const ImU32 armorCol = g_cfg.armorcol.to_color().as_imcolor();
    const ImU32 textCol = g_cfg.distcol.to_color().as_imcolor();
    const ImU32 skelCol = g_cfg.skelcol.to_color().as_imcolor();
    const ImU32 snapCol = g_cfg.snapcol.to_color().as_imcolor();

    const int poolSize = CPools::ms_pPedPool->m_nSize;
    CMatrix& camM = TheCamera.m_mCameraMatrix;

    for (int i = 0; i < poolSize; i++)
    {
        CPed* ped = CPools::ms_pPedPool->GetAt(i);

        if (!ped || ped == pLocal || ped->m_fHealth <= 0.0f)
        {
            continue;
        }

        const CVector foot = ped->GetPosition();
        const float dist = VecLength(CVector(foot.x - localPos.x, foot.y - localPos.y, foot.z - localPos.z));

        if (dist > g_cfg.whDistance)
        {
            continue;
        }

        if (g_cfg.wh_flags & WH_SNAP)
        {
            const RwV3d epos = { foot.x, foot.y, foot.z - 1.1f };
            RwV3d escr{};
            float ew = 0.0f, eh = 0.0f;
            ImVec2 to;

            if (CSprite::CalcScreenCoors(epos, &escr, &ew, &eh, true, true))
            {
                to = ImVec2(escr.x, escr.y);
            }
            else
            {
                CMatrix& cm = TheCamera.m_mCameraMatrix;
                float dx = foot.x - cm.pos.x;
                float dy = foot.y - cm.pos.y;
                float dz = foot.z - cm.pos.z;
                float rx = dx * cm.right.x + dy * cm.right.y + dz * cm.right.z;
                float fx = dx * cm.at.x + dy * cm.at.y + dz * cm.at.z;
                float ang = atan2f(rx, fx);
                float cx = ImGui::GetIO().DisplaySize.x * 0.5f;
                float cy = ImGui::GetIO().DisplaySize.y * 0.5f;
                float rr = (cx < cy ? cx : cy) * 0.9f;
                to = ImVec2(cx + sinf(ang) * rr, cy - cosf(ang) * rr);
            }

            const RwV3d lpos = { localPos.x, localPos.y, localPos.z - 1.1f };
            RwV3d lscr{};
            float lw = 0.0f, lh = 0.0f;
            ImVec2 from;

            if (CSprite::CalcScreenCoors(lpos, &lscr, &lw, &lh, true, true))
            {
                from = ImVec2(lscr.x, lscr.y);
            }
            else
            {
                from = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y);
            }

            draw->AddLine(from, to, snapCol, 2.0f);
        }

        {
            float tx = foot.x - camM.pos.x;
            float ty = foot.y - camM.pos.y;
            float tz = foot.z - camM.pos.z;

            if (tx * camM.at.x + ty * camM.at.y + tz * camM.at.z <= 0.0f)
            {
                continue;
            }
        }

        float pedH = 1.8f;
        float footZ = foot.z;

        if (CColModel* col = ped->GetColModel())
        {
            const float h = col->m_boundBox.m_vecMax.z - col->m_boundBox.m_vecMin.z;

            if (h >= 0.5f && h <= 3.0f)
            {
                pedH = h;
                footZ = foot.z + col->m_boundBox.m_vecMin.z;
            }
        }

        const RwV3d foot3d = { foot.x, foot.y, footZ };
        const RwV3d head3d = { foot.x, foot.y, footZ + pedH };
        RwV3d footScr{}, headScr{};
        float w = 0.0f, h = 0.0f;

        if (!CSprite::CalcScreenCoors(foot3d, &footScr, &w, &h, false, false))
        {
            continue;
        }

        if (!CSprite::CalcScreenCoors(head3d, &headScr, &w, &h, false, false))
        {
            continue;
        }

        const float boxH = footScr.y - headScr.y;

        if (boxH <= 0.0f)
        {
            continue;
        }

        const float boxW = boxH * 0.45f;
        const ImVec2 min(headScr.x - boxW * 0.5f, headScr.y);
        const ImVec2 max(headScr.x + boxW * 0.5f, footScr.y);

        if (g_cfg.wh_flags & WH_BOX)
        {
            draw->AddRect(min - ImVec2(1.0f, 1.0f), max + ImVec2(1.0f, 1.0f), backCol, 0.0f, 0, 1.0f);
            draw->AddRect(min, max, boxCol, 0.0f, 0, 1.0f);
            draw->AddRect(min + ImVec2(1.0f, 1.0f), max - ImVec2(1.0f, 1.0f), backCol, 0.0f, 0, 1.0f);
        }

        if (g_cfg.wh_flags & WH_HP)
        {
            const float hp = std::clamp(ped->m_fHealth / 100.0f, 0.0f, 1.0f);
            const ImVec2 barMin(min.x - 5.0f * s, min.y);
            const ImVec2 barMax(min.x - 2.0f * s, max.y);
            const ImU32 hpFill = g_cfg.hpcol.to_color().multiply(c_color(255, 30, 30), 1.0f - hp).as_imcolor();
            draw->AddRect(barMin - ImVec2(1.0f, 1.0f), barMax + ImVec2(1.0f, 1.0f), backCol, 0.0f, 0, 1.0f);
            draw->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 150), 0.0f);
            draw->AddRectFilled(ImVec2(barMin.x, barMax.y - (barMax.y - barMin.y) * hp), barMax, hpFill, 0.0f);
        }

        if ((g_cfg.wh_flags & WH_ARMOR) && ped->m_fArmour > 0.0f)
        {
            const float ap = std::clamp(ped->m_fArmour / 100.0f, 0.0f, 1.0f);
            const ImVec2 barMin(max.x + 2.0f * s, min.y);
            const ImVec2 barMax(max.x + 5.0f * s, max.y);
            draw->AddRect(barMin - ImVec2(1.0f, 1.0f), barMax + ImVec2(1.0f, 1.0f), backCol, 0.0f, 0, 1.0f);
            draw->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 150), 0.0f);
            draw->AddRectFilled(ImVec2(barMin.x, barMax.y - (barMax.y - barMin.y) * ap), barMax, armorCol, 0.0f);
        }

        if (g_cfg.wh_flags & WH_DIST)
        {
            char buf[16]{};
            snprintf(buf, sizeof(buf), "%d m", (int)dist);
            const ImVec2 tp(min.x, max.y + 2.0f * s);
            draw->AddText(tp + ImVec2(1.0f, 1.0f), IM_COL32(0, 0, 0, 200), buf);
            draw->AddText(tp, textCol, buf);
        }

        if (g_cfg.wh_flags & WH_SKELETON)
        {
            static const int segs[][2] = {
                { BONE_PELVIS, BONE_SPINE1 }, { BONE_SPINE1, BONE_UPPERTORSO },
                { BONE_UPPERTORSO, BONE_NECK }, { BONE_NECK, BONE_HEAD },
                { BONE_UPPERTORSO, BONE_RIGHTSHOULDER }, { BONE_RIGHTSHOULDER, BONE_RIGHTELBOW },
                { BONE_RIGHTELBOW, BONE_RIGHTWRIST },
                { BONE_UPPERTORSO, BONE_LEFTSHOULDER }, { BONE_LEFTSHOULDER, BONE_LEFTELBOW },
                { BONE_LEFTELBOW, BONE_LEFTWRIST },
                { BONE_PELVIS, BONE_RIGHTHIP }, { BONE_RIGHTHIP, BONE_RIGHTKNEE },
                { BONE_RIGHTKNEE, BONE_RIGHTANKLE },
                { BONE_PELVIS, BONE_LEFTHIP }, { BONE_LEFTHIP, BONE_LEFTKNEE },
                { BONE_LEFTKNEE, BONE_LEFTANKLE },
            };

            for (int si = 0; si < 16; si++)
            {
                RwV3d bw0{}, bw1{};

                ped->GetBonePosition(bw0, (unsigned int)segs[si][0], true);
                ped->GetBonePosition(bw1, (unsigned int)segs[si][1], true);

                RwV3d bs0{}, bs1{};
                float sw = 0.0f, sh = 0.0f;

                if (!CSprite::CalcScreenCoors(bw0, &bs0, &sw, &sh, false, false))
                {
                    continue;
                }

                if (!CSprite::CalcScreenCoors(bw1, &bs1, &sw, &sh, false, false))
                {
                    continue;
                }

                draw->AddLine(ImVec2(bs0.x, bs0.y), ImVec2(bs1.x, bs1.y), skelCol, 1.0f);
            }
        }

    }
}
