// language: C++, file: entity.h
#pragma once
#include "memory.h"
#include "offsets.h"
#include "math.h"
#include <string>
#include <array>

struct BoneMatrix { float m[3][4]; }; // 3x4 per bone

struct Player {
    uintptr_t pawn_ptr   = 0;
    uintptr_t scene_node = 0;
    Vec3      origin     = {};
    Vec3      head       = {};
    int       health     = 0;
    int       team       = 0;
    bool      dormant    = false;
    float     distance   = 0.f;
    std::array<Vec3, 32> bones = {};

    bool valid() const { return pawn_ptr && health > 0 && health <= 100 && !dormant; }
};

// read bone world position from model state
inline Vec3 get_bone_pos(Memory& mem, uintptr_t scene_node, int bone_idx) {
    uintptr_t model_state = mem.read<uintptr_t>(scene_node + offsets::m_modelState);
    uintptr_t bone_array  = mem.read<uintptr_t>(model_state + 0x80);
    BoneMatrix bm = mem.read<BoneMatrix>(bone_array + bone_idx * sizeof(BoneMatrix));
    return { bm.m[0][3], bm.m[1][3], bm.m[2][3] };
}
