// language: C++, file: math.h
#pragma once
#include <cmath>
#include <numbers>
#include <algorithm>

struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float length2d() const { return std::sqrt(x*x + y*y); }
};

struct Matrix4x4 { float m[4][4]; };

inline Vec2 world_to_screen(const Vec3& world, const Matrix4x4& vm, int sw, int sh) {
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];
    if (w < 0.001f) return {-1.f, -1.f};
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    return {
        (sw / 2.f) * (1.f + x / w),
        (sh / 2.f) * (1.f - y / w)
    };
}

inline Vec2 calc_angle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = dst - src;
    float yaw   = std::atan2(delta.y, delta.x) * (180.f / std::numbers::pi_v<float>);
    float pitch = std::atan2(-delta.z, delta.length2d()) * (180.f / std::numbers::pi_v<float>);
    return {pitch, yaw};
}

inline float fov_distance(const Vec2& a_ang, const Vec2& b_ang) {
    float dp = a_ang.x - b_ang.x;
    float dy = a_ang.y - b_ang.y;
    return std::sqrt(dp*dp + dy*dy);
}
