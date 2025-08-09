// main.cpp
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <Shlwapi.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "audio/AudioCapture.h"
#include "utils/VUBuffer.h"
#include "utils/fonts.h"
#include "gui/EQSwitchWindow.h"
#include "utils/EqualizerAPOManager.h"
#include "utils/Theme.h"
#include "resource.h"

// ---------------- Global Direct3D state ----------------
static ID3D11Device*           g_pd3dDevice           = nullptr;
static ID3D11DeviceContext*    g_pd3dDeviceContext    = nullptr;
static IDXGISwapChain*         g_pSwapChain           = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static HWND                    g_hWnd                 = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// ---------------- Global App State ----------------
auto& apoManager      = EqualizerAPOManager::getInstance();
std::string configDir = apoManager.getConfigDir();
std::string profilesDir = apoManager.getProfilesDir();
std::string configTarget = configDir + "\\config.txt";

// ---------------- Function Declarations ----------------
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
bool InitAppWindow(HINSTANCE hInstance, float scale);
void InitImGui(float scale);
void MainLoop(float scale);
void ShutdownApp(WNDCLASSEX& wc);

// ---------------- D3D Functions ----------------
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevelArray[] = {D3D_FEATURE_LEVEL_11_0};
    if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            featureLevelArray, 1, D3D11_SDK_VERSION,
            &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pd3dDeviceContext)))
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain)         { g_pSwapChain->Release();         g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext)  { g_pd3dDeviceContext->Release();  g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice)         { g_pd3dDevice->Release();         g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// ---------------- Win32 Window Procedure ----------------
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0; // Disable ALT menu
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ---------------- Window Initialization ----------------
bool InitAppWindow(HINSTANCE hInstance, float scale) {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                     hInstance, nullptr, nullptr, nullptr, nullptr,
                     _T("EQ Switch"), nullptr};
    RegisterClassEx(&wc);

    g_hWnd = CreateWindow(wc.lpszClassName, _T("EQ Switch"),
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          (int)(800 * scale), (int)(700 * scale),
                          nullptr, nullptr, wc.hInstance, nullptr);

    // Set icon
    if (HICON hIcon = (HICON)LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1))) {
        SendMessageW(g_hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        SendMessageW(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    if (!CreateDeviceD3D(g_hWnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);
    return true;
}

// ---------------- ImGui Initialization ----------------
void InitImGui(float scale) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    LoadFonts();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;

    ApplyVSCodeDarkModernTheme();
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

// ---------------- Main Loop ----------------
void MainLoop(float scale) {
    if (apoManager.detectInstallation()) {
        configTarget = apoManager.getConfigDir() + "\\config.txt";
    }

    constexpr int NUM_CHANNELS = 6;
    VUBuffer gVUBuffer(NUM_CHANNELS);
    AudioCapture gAudioCapture(gVUBuffer);
    ProfileManager gProfileManager(profilesDir, configTarget);

    gAudioCapture.start();

    bool exitApp = false;
    MSG msg = {};
    while (msg.message != WM_QUIT && !exitApp) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ShowEQSwitchWindow(gProfileManager, gVUBuffer, &exitApp, scale, apoManager);

        ImGui::Render();
        const float clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    gAudioCapture.stop();
}

// ---------------- Shutdown ----------------
void ShutdownApp(WNDCLASSEX& wc) {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

// ---------------- Entry Point ----------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
#ifndef NDEBUG
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif
    ImGui_ImplWin32_EnableDpiAwareness();
    float scale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY));

    if (!InitAppWindow(hInstance, scale)) return 1;
    InitImGui(scale);
    MainLoop(scale);

    // Dummy WNDCLASSEX for cleanup
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance};
    ShutdownApp(wc);
    return 0;
}
// ---------------- End of main.cpp ----------------