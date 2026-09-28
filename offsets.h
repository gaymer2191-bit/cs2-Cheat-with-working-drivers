// language: C++, file: offsets.h
// CS2 client.dll offsets — RE-DUMP after every game update
// tools: cs2-dumper (github.com/a2x/cs2-dumper), hazedumper CS2 fork

#pragma once
#include <cstdint>

namespace offsets {
    // client.dll
    inline constexpr uintptr_t dwEntityList    = 0x18C4700; // IGameEntitySystem
    inline constexpr uintptr_t dwLocalPlayer   = 0x173C4B8; // C_CSPlayerPawn*
    inline constexpr uintptr_t dwViewMatrix    = 0x19187C0; // float[4][4]
    inline constexpr uintptr_t dwPlantedC4     = 0x18C5148;

    // C_BaseEntity
    inline constexpr uintptr_t m_iHealth        = 0x334;
    inline constexpr uintptr_t m_iTeamNum       = 0x3CB;
    inline constexpr uintptr_t m_bDormant       = 0xE8;    // via m_pGameSceneNode
    inline constexpr uintptr_t m_vecAbsOrigin   = 0x80;    // on CGameSceneNode

    // C_CSPlayerPawn
    inline constexpr uintptr_t m_pGameSceneNode = 0x328;
    inline constexpr uintptr_t m_vOldOrigin     = 0x1224;
    inline constexpr uintptr_t m_iShotsFired    = 0x1474;
    inline constexpr uintptr_t m_aimPunchAngle  = 0x1470;
    inline constexpr uintptr_t m_flFlashDuration= 0x1244;

    // CGameSceneNode
    inline constexpr uintptr_t m_modelState     = 0x170; // CModelState
    // bone array: m_modelState + 0x80 → ptr to bone transforms (3x4 matrix per bone)

    // bone indices (CS2)
    inline constexpr int BONE_HEAD   = 6;
    inline constexpr int BONE_NECK   = 5;
    inline constexpr int BONE_CHEST  = 4;
    inline constexpr int BONE_PELVIS = 0;
}
