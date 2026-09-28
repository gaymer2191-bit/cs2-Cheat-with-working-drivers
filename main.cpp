// language: C++, file: main.cpp, target: Windows 11, MSVC
#include "memory.h"
#include "entity.h"
#include "aimbot.h"
#include "esp.h"
#include "overlay.h"
#include "offsets.h"
#include "math.h"
#include <imgui.h>
#include <vector>
#include <thread>
#include <chrono>

static bool g_esp_enabled    = true;
static bool g_aimbot_enabled = true;
static bool g_menu_open      = false;

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Memory mem(L"cs2.exe");
    if (!mem.base) return 1;

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    if (!overlay_init(sw, sh)) return 2;

    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessageW(&msg, nullptr, 0,0, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessageW(&msg); continue;
        }

        // toggle menu
        if (GetAsyncKeyState(VK_INSERT) & 1) g_menu_open ^= true;
        if (GetAsyncKeyState(VK_END)    & 1) break;

        // read view matrix
        Matrix4x4 vm = mem.read<Matrix4x4>(mem.base + offsets::dwViewMatrix);

        // read local player
        uintptr_t local_pawn = mem.read<uintptr_t>(mem.base + offsets::dwLocalPlayer);
        Player local{};
        local.pawn_ptr   = local_pawn;
        local.scene_node = mem.read<uintptr_t>(local_pawn + offsets::m_pGameSceneNode);
        local.origin     = mem.read<Vec3>(local.scene_node + offsets::m_vecAbsOrigin);
        local.head       = get_bone_pos(mem, local.scene_node, offsets::BONE_HEAD);
        local.team       = mem.read<int>(local_pawn + offsets::m_iTeamNum);
        local.health     = mem.read<int>(local_pawn + offsets::m_iHealth);

        // view angles — read from clientstate or engine (offset varies, derive per build)
        Vec2 view_angles = mem.read<Vec2>(mem.base + 0x1873348); // re-derive per patch

        // entity list walk
        uintptr_t entity_list = mem.read<uintptr_t>(mem.base + offsets::dwEntityList);
        std::vector<Player> players;
        players.reserve(64);

        for (int i = 1; i < 64; i++) {
            uintptr_t list_entry = mem.read<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
            if (!list_entry) continue;
            uintptr_t pawn = mem.read<uintptr_t>(list_entry + 120 * (i & 0x1FF));
            if (!pawn || pawn == local_pawn) continue;

            Player p{};
            p.pawn_ptr   = pawn;
            p.scene_node = mem.read<uintptr_t>(pawn + offsets::m_pGameSceneNode);
            if (!p.scene_node) continue;
            p.health  = mem.read<int>(pawn  + offsets::m_iHealth);
            p.team    = mem.read<int>(pawn  + offsets::m_iTeamNum);
            p.dormant = mem.read<bool>(p.scene_node + offsets::m_bDormant);
            p.origin  = mem.read<Vec3>(p.scene_node + offsets::m_vecAbsOrigin);
            p.head    = get_bone_pos(mem, p.scene_node, offsets::BONE_HEAD);
            p.distance = (p.origin - local.origin).length();

            // read relevant bones for skeleton
            for (int b = 0; b < 20; b++)
                p.bones[b] = get_bone_pos(mem, p.scene_node, b);

            players.push_back(p);
        }

        // aimbot
        if (g_aimbot_enabled)
            aimbot_tick(mem, players, local, view_angles, sw, sh);

        // render frame
        overlay_begin_frame();
        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        if (g_esp_enabled)
            esp_draw(dl, players, local, vm, sw, sh);

        // menu
        if (g_menu_open) {
            ImGui::SetNextWindowSize({320, 220}, ImGuiCond_Once);
            ImGui::SetNextWindowPos({40, 40}, ImGuiCond_Once);
            ImGui::Begin("VANTA CS2", nullptr, ImGuiWindowFlags_NoCollapse);
            ImGui::Checkbox("ESP",    &g_esp_enabled);
            ImGui::Checkbox("Aimbot", &g_aimbot_enabled);
            ImGui::Separator();
            ImGui::Text("INSERT = menu  |  END = exit");
            ImGui::End();
        }

        overlay_end_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    overlay_shutdown();
    return 0;
}
