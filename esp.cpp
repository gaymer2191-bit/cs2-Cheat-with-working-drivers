// language: C++, file: esp.cpp
#include "esp.h"
#include "math.h"
#include <imgui.h>
#include <vector>
#include <format>

static ImU32 health_color(int hp) {
    float t = hp / 100.f;
    return IM_COL32(
        (int)((1.f - t) * 255),
        (int)(t * 255),
        60, 220
    );
}

void esp_draw(
    ImDrawList* dl,
    const std::vector<Player>& players,
    const Player& local,
    const Matrix4x4& vm,
    int sw, int sh
) {
    for (auto& p : players) {
        if (!p.valid() || p.team == local.team) continue;

        Vec2 head_s = world_to_screen(p.head, vm, sw, sh);
        Vec2 feet_s = world_to_screen(p.origin, vm, sw, sh);

        if (head_s.x < 0 || feet_s.x < 0) continue;

        float height = feet_s.y - head_s.y;
        float width  = height * 0.45f;
        float cx     = head_s.x;

        // 2D bounding box
        dl->AddRect(
            {cx - width, head_s.y},
            {cx + width, feet_s.y},
            IM_COL32(0,0,0,180), 0.f, 0, 3.f
        );
        dl->AddRect(
            {cx - width + 1, head_s.y + 1},
            {cx + width - 1, feet_s.y - 1},
            IM_COL32(255,255,255,200)
        );

        // health bar — left side
        float bar_h = height * (p.health / 100.f);
        dl->AddRectFilled(
            {cx - width - 6, feet_s.y - bar_h},
            {cx - width - 2, feet_s.y},
            health_color(p.health)
        );

        // skeleton
        struct BonePair { int a, b; };
        static constexpr BonePair skeleton[] = {
            {6,5},{5,4},{4,0},         // head→neck→chest→pelvis
            {4,10},{10,11},{11,12},    // chest→L shoulder→elbow→wrist
            {4,7},{7,8},{8,9},         // chest→R shoulder→elbow→wrist
            {0,16},{16,17},{17,18},    // pelvis→L hip→knee→ankle
            {0,13},{13,14},{14,15}     // pelvis→R hip→knee→ankle
        };
        for (auto& bp : skeleton) {
            if (bp.a >= (int)p.bones.size() || bp.b >= (int)p.bones.size()) continue;
            Vec2 sa = world_to_screen(p.bones[bp.a], vm, sw, sh);
            Vec2 sb = world_to_screen(p.bones[bp.b], vm, sw, sh);
            if (sa.x < 0 || sb.x < 0) continue;
            dl->AddLine({sa.x,sa.y},{sb.x,sb.y}, IM_COL32(255,255,255,180), 1.5f);
        }

        // distance label
        std::string dist_str = std::format("{:.0f}m", p.distance / 100.f);
        dl->AddText({cx - width, head_s.y - 14.f}, IM_COL32(220,220,220,255), dist_str.c_str());
    }
}
