// language: C++, file: aimbot.cpp
#include "aimbot.h"
#include "math.h"
#include <windows.h>
#include <vector>
#include <algorithm>
#include <numbers>

// config
static constexpr float FOV_LIMIT    = 8.f;   // degrees — reduce to tighten cone
static constexpr float SMOOTH       = 6.f;   // higher = slower, more human
static constexpr bool  RCS_ENABLED  = true;
static constexpr float RCS_SCALE    = 0.85f;

static void send_mouse_delta(float dx, float dy) {
    INPUT in{};
    in.type           = INPUT_MOUSE;
    in.mi.dwFlags     = MOUSEEVENTF_MOVE;
    in.mi.dx          = (LONG)dx;
    in.mi.dy          = (LONG)dy;
    SendInput(1, &in, sizeof(INPUT));
    // for driver-level: write dx/dy to HID shared mem instead
}

void aimbot_tick(
    Memory& mem,
    const std::vector<Player>& players,
    const Player& local,
    const Vec2& view_angles,   // current camera pitch/yaw
    int screen_w, int screen_h
) {
    if (!(GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) return; // hold mouse5 to aim

    Player const* best = nullptr;
    float best_fov = FOV_LIMIT;

    for (auto& p : players) {
        if (!p.valid() || p.team == local.team) continue;
        Vec2 target_ang = calc_angle(local.head, p.head);
        float fov = fov_distance(view_angles, target_ang);
        if (fov < best_fov) { best_fov = fov; best = &p; }
    }

    if (!best) return;

    Vec2 target_ang = calc_angle(local.head, best->head);

    // recoil compensation — subtract punch angle from target
    Vec2 aim_ang = target_ang;
    if (RCS_ENABLED) {
        Vec2 punch = mem.read<Vec2>(local.pawn_ptr + offsets::m_aimPunchAngle);
        aim_ang.x -= punch.x * RCS_SCALE * 2.f;
        aim_ang.y -= punch.y * RCS_SCALE * 2.f;
    }

    float dp = aim_ang.x - view_angles.x;
    float dy = aim_ang.y - view_angles.y;

    // wrap yaw
    while (dy > 180.f) dy -= 360.f;
    while (dy < -180.f) dy += 360.f;

    // convert angle delta to pixel delta (sensitivity dependent — 0.022 * sens)
    constexpr float SENS_FACTOR = 0.022f * 1.0f; // set to your in-game sensitivity
    float px = (dy / SENS_FACTOR) / SMOOTH;
    float py = (dp / SENS_FACTOR) / SMOOTH;

    send_mouse_delta(px, py);
}
