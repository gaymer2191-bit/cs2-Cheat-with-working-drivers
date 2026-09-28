// language: C++, file: overlay.cpp
// D3D11 transparent topmost window, ImGui rendered on top
#include "overlay.h"
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <dwmapi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

static HWND             g_hwnd    = nullptr;
static ID3D11Device*    g_device  = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain*  g_swap    = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
static LRESULT WINAPI WndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hw,msg,wp,lp)) return true;
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(hw,msg,wp,lp);
}

bool overlay_init(int sw, int sh) {
    WNDCLASSEXW wc{ sizeof(wc), CS_CLASSDC, WndProc, 0,0, GetModuleHandle(nullptr),
                    nullptr,nullptr,nullptr,nullptr, L"cs2_vanta", nullptr };
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        L"cs2_vanta", L"", WS_POPUP,
        0, 0, sw, sh, nullptr, nullptr, wc.hInstance, nullptr
    );

    // fully transparent — click-through
    SetLayeredWindowAttributes(g_hwnd, 0, 0, LWA_ALPHA);
    MARGINS m{-1}; DwmExtendFrameIntoClientArea(g_hwnd, &m);
    ShowWindow(g_hwnd, SW_SHOW); UpdateWindow(g_hwnd);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = sw; sd.BufferDesc.Height = sh;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl;
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &g_swap, &g_device, &fl, &g_ctx
    ))) return false;

    ID3D11Texture2D* back = nullptr;
    g_swap->GetBuffer(0, IID_PPV_ARGS(&back));
    g_device->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = nullptr;

    // custom style — clean dark
    auto& s = ImGui::GetStyle();
    s.WindowBorderSize = 0; s.FrameRounding = 4; s.GrabRounding = 4;
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]        = {0.07f,0.07f,0.09f,0.85f};
    c[ImGuiCol_FrameBg]         = {0.12f,0.12f,0.16f,1.f};
    c[ImGuiCol_SliderGrab]      = {0.36f,0.55f,0.95f,1.f};
    c[ImGuiCol_CheckMark]       = {0.36f,0.55f,0.95f,1.f};
    c[ImGuiCol_Button]          = {0.2f,0.3f,0.7f,0.9f};

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_device, g_ctx);
    return true;
}

void overlay_begin_frame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void overlay_end_frame() {
    ImGui::Render();
    float clear[4]{0,0,0,0};
    g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
    g_ctx->ClearRenderTargetView(g_rtv, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_swap->Present(1, 0);
}

void overlay_shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_rtv)  g_rtv->Release();
    if (g_swap) g_swap->Release();
    if (g_ctx)  g_ctx->Release();
    if (g_device) g_device->Release();
    DestroyWindow(g_hwnd);
}

HWND overlay_hwnd() { return g_hwnd; }
