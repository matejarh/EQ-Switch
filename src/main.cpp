#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
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
#include <iostream>
#include <Shlwapi.h>

// Global Direct3D device
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static HWND g_hWnd = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global Equalizer APO manager instance
auto &apoManager = EqualizerAPOManager::getInstance();
// Default global configuration paths
std::string configDir = "C:\\Program Files\\EqualizerAPO\\config";
// std::string profilesDir = "..\\eq-presets\\";
std::string profilesDir = apoManager.getProfilesDir();
std::string configTarget = apoManager.getConfigDir() + "\\config.txt";

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

/* #include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h> // for Microsoft::WRL::ComPtr
using Microsoft::WRL::ComPtr;

// Helper function to create a shader resource from icon resource
bool CreateTextureFromIconResource(ID3D11Device *device, HINSTANCE hInstance, int resourceID, ID3D11ShaderResourceView **outSRV)
{
    // Load icon from resource
    HICON hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(resourceID));
    if (!hIcon)
        return false;

    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo))
        return false;

    BITMAP bmpColor;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpColor);

    int width = bmpColor.bmWidth;
    int height = bmpColor.bmHeight;

    // Create D3D11 texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(hr))
        return false;

    // Copy pixel data from icon to texture
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);
    context->Map(texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    // Create compatible DC and copy pixels
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, iconInfo.hbmColor);
    BitBlt(hdcMem, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
    GetBitmapBits(iconInfo.hbmColor, width * height * 4, mapped.pData);
    SelectObject(hdcMem, hOldBmp);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    context->Unmap(texture.Get(), 0);

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, outSRV);
    return SUCCEEDED(hr);
} */

// Direct3D and Win32 helpers
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[1] = {D3D_FEATURE_LEVEL_11_0};

    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                      createDeviceFlags, featureLevelArray, 1,
                                      D3D11_SDK_VERSION, &sd, &g_pSwapChain,
                                      &g_pd3dDevice, &featureLevel,
                                      &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext)
    {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#ifndef NDEBUG
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                     hInstance, nullptr, nullptr, nullptr, nullptr,
                     _T("EQ Switch"), nullptr};
    RegisterClassEx(&wc);
    
    g_hWnd = CreateWindow(wc.lpszClassName, _T("EQ Switch"),
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (int)(800 * main_scale), (int)(700 * main_scale),
                          nullptr, nullptr, wc.hInstance, nullptr);

    // Set window icon (small and big)
    HICON hIcon = (HICON)LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon)
    {
        SendMessageW(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessageW(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
    // Initialize Direct3D
    if (!CreateDeviceD3D(g_hWnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    LoadFonts();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Dear ImGui style
    // ApplyEQSwitchDarkTheme();
    ApplyVSCodeDarkModernTheme();

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    /* ID3D11ShaderResourceView *iconTexture = nullptr;
    bool iconLoaded = CreateTextureFromIconResource(g_pd3dDevice, hInstance, IDI_ICON1, &iconTexture);

    if (!iconLoaded)
    {
        std::cout << "Failed to load icon resource" << "\n";
    } */

    // Initialize Equalizer APO manager
    if (apoManager.detectInstallation())
    {
        configTarget = apoManager.getConfigDir() + "\\config.txt";
    }
    // Global VU buffer and audio capture
    constexpr int NUM_CHANNELS = 6;
    VUBuffer gVUBuffer(NUM_CHANNELS);
    AudioCapture gAudioCapture(gVUBuffer);
    ProfileManager gProfileManager(profilesDir, configTarget);

    // Start audio capture
    gAudioCapture.start();

    // Main loop
    bool exitApp = false;
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT && !exitApp)
    {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ShowEQSwitchWindow(gProfileManager, gVUBuffer, &exitApp, main_scale, apoManager/* , iconTexture */);

        ImGui::Render();
        const float clear_color_with_alpha[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    gAudioCapture.stop();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    /* if (iconTexture)
        iconTexture->Release(); */

    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}