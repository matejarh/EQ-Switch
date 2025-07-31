// main.cpp
// This file is part of the EQ Switch project, which provides a GUI for managing Equalizer APO profiles.

// ImGui/Win32/DX11 includes
#include <windows.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// EQ-switch.cpp declarations
#include <string>
#include <vector>

#include "resources/resource.h"

// Additional includes for audio capture
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>
#include <cmath>
#include <comdef.h>
#include <wrl/client.h> // For Microsoft::WRL::ComPtr
#include <algorithm>
#include <iostream>
#include <mutex>
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Avrt.lib")
#pragma comment(lib, "d3d11.lib")

// Forward declarations for EQ-switch.cpp functions/structs
struct Profile
{
    std::string label;
    std::string filename;
};
std::vector<Profile> loadProfiles(const std::string &profilesDir);
std::string getCurrentProfile(const std::string &configFilePath);
bool copyFile(const std::string &src, const std::string &dest);
void restartEqualizerAPO();
bool launchEditor();

const char *channelLabels[6] = {"L", "R", "C", "LFE", "LS", "RS"};

const std::string configDir = "C:\\Program Files\\EqualizerAPO\\config\\";
const std::string profilesDir = "..\\eq-presets\\";
const std::string configTarget = configDir + "config.txt";

// Number of channels for VU meters
constexpr int NUM_CHANNELS = 6; // Change as needed
std::vector<float> vuBuffer(NUM_CHANNELS, 0.0f);
std::mutex vuMutex;

ImFont *g_SmallFont = nullptr;

// Helper: Compute RMS (VU) for a buffer of float samples
float ComputeRMS(const float *samples, size_t count)
{
    double sum = 0.0;
    for (size_t i = 0; i < count; ++i)
        sum += samples[i] * samples[i];
    return (float)sqrt(sum / count);
}

// Capture audio loop
// This function captures audio from the default audio device and computes VU values for each channel
void CaptureAudioLoop(int numChannels)
{
    Microsoft::WRL::ComPtr<IMMDevice> device;
    Microsoft::WRL::ComPtr<IAudioClient> audioClient;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient;
    WAVEFORMATEX *pwfx = nullptr;
    UINT32 bufferFrameCount = 0;

    const float channelGains[6] = {4.0f, 4.0f, 9.0f, 3.0f, 4.0f, 4.0f};
    std::vector<float> lastVu(numChannels, 0.0f);

    HRESULT hr;

    // Init COM
    CoInitialize(nullptr);

    // Get default audio device
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    if (FAILED(hr))
        return;

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr))
        return;

    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&audioClient);
    if (FAILED(hr))
        return;

    hr = audioClient->GetMixFormat(&pwfx);
    if (FAILED(hr))
        return;

    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        1000000, 0,
        pwfx, nullptr);
    if (FAILED(hr))
        return;

    hr = audioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr))
        return;

    hr = audioClient->GetService(IID_PPV_ARGS(&captureClient));
    if (FAILED(hr))
        return;

    hr = audioClient->Start();
    if (FAILED(hr))
        return;

    // --- Main capture loop ---
    while (true)
    {
        UINT32 packetLength = 0;
        hr = captureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr) || packetLength == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        while (packetLength != 0)
        {
            BYTE *pData;
            UINT32 numFramesAvailable;
            DWORD flags;
            hr = captureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr))
                break;

            if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE || pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
            {
                float *samples = (float *)pData;
                std::vector<std::vector<float>> channelSamples(numChannels);

                for (UINT32 i = 0; i < numFramesAvailable; ++i)
                {
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        channelSamples[ch].push_back(samples[i * numChannels + ch]);
                    }
                }

                std::lock_guard<std::mutex> lock(vuMutex);
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float rms = ComputeRMS(channelSamples[ch].data(), channelSamples[ch].size());
                    rms *= channelGains[ch]; // boost

                    float smoothed = lastVu[ch] + (rms - lastVu[ch]) * 0.1f;
                    smoothed = std::clamp(smoothed, 0.0f, 1.0f);
                    vuBuffer[ch] = smoothed;
                    lastVu[ch] = smoothed;
                }
            }

            captureClient->ReleaseBuffer(numFramesAvailable);
            captureClient->GetNextPacketSize(&packetLength);
        }
    }

    CoUninitialize(); // if needed
}

// Get current VU values for all channels
// Returns a vector of VU values, one for each channel
std::vector<float> GetCurrentVUs(int numChannels)
{
    std::vector<float> vuValues(numChannels, 0.0f);
    std::lock_guard<std::mutex> lock(vuMutex);
    for (int i = 0; i < numChannels && i < vuBuffer.size(); ++i)
    {
        vuValues[i] = vuBuffer[i];
    }
    return vuValues;
}

// Get VU value for a specific channel
// Returns the VU value for the specified channel, or 0 if the channel index is out of bounds
float GetChannelVU(int channel)
{
    static int lastChannelCount = 6; // fallback
    std::vector<float> vu = GetCurrentVUs(lastChannelCount);

    if (vu.size() > channel)
        return vu[channel];
    else if (!vu.empty())
        lastChannelCount = vu.size();
    return 0.0f;
}

// Apply a profile to the EQ Switch
// This function applies the specified profile by copying the profile file to the target config location
static std::vector<Profile> profiles = loadProfiles(profilesDir);
static std::string currentProfile = getCurrentProfile(configTarget);

static int selectedProfile = 0;

void applyProfile(const Profile &p, bool *p_exit = nullptr)
{
    std::string srcFile = profilesDir + p.filename;
    if (copyFile(srcFile, configTarget))
    {
        restartEqualizerAPO();
        currentProfile = getCurrentProfile(configTarget);
    }
}

// Show the EQ Switch window with VU meters and profile selection
// This function creates an ImGui window that displays VU meters for each channel and allows profile
// selection from a list of loaded profiles
// If p_exit is provided, clicking the "Exit" profile will set it to true
void ShowEQSwitchWindow(bool *p_exit = nullptr)
{

    for (int i = 0; i < profiles.size(); ++i)
    {
        if (profiles[i].label == currentProfile)
        {
            selectedProfile = i;
            break;
        }
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("EQ Switch", p_exit, window_flags);
    // ImGui::Begin("EQ Switch", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Current Active Profile: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", currentProfile.c_str());

    // VU meters section
    ImGui::Separator();
    ImGui::Text("VU Meters:");
    ImGui::BeginGroup();

    const int numLeds = 8;
    const ImVec2 ledSize(30, 20);                                           // LED width x height
    const float ledSpacing = 6.0f;                                          // Gap between LEDs
    const float vuHeight = numLeds * (ledSize.y + ledSpacing) - ledSpacing; // Total height

    for (int ch = 0; ch < NUM_CHANNELS; ++ch)
    {
        float vu = GetChannelVU(ch);
        int activeLeds = static_cast<int>(vu * numLeds + 0.5f);

        // Begin full column
        ImGui::BeginGroup();
        
        ImGui::PushID(ch); // Unique ID for each channel group

        // Calculate position for LEDs
        ImVec2 groupPos = ImGui::GetCursorScreenPos();
        float columnWidth = ledSize.x + ledSpacing; // Width of each column including spacing
        float columnXCenter = groupPos.x + columnWidth * 0.5f;

        // Determine label
        const char *label = (ch < 6) ? channelLabels[ch] : "Ch?";
        ImVec2 labelSize = ImGui::CalcTextSize(label);

        // Position label centered above column
        ImVec2 labelPos(columnXCenter - labelSize.x * 0.5f, groupPos.y);

        // Draw label manually (so it's pixel-accurate)
        ImGui::GetWindowDrawList()->AddText(g_SmallFont, 20.0f, labelPos, IM_COL32(160, 160, 160, 255), label);

        // Reserve space so LED drawing starts below label
        ImGui::Dummy(ImVec2(columnWidth, labelSize.y + 1.0f)); // add vertical spacing

        // --- LED stack with fixed size container ---
        ImVec2 ledStackPos = ImGui::GetCursorScreenPos();
        ImVec2 ledStackSize = ImVec2(ledSize.x, vuHeight);

        ImGui::InvisibleButton("##vu_bg", ledStackSize); // Reserve space
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetItemRectMin();

        for (int i = 0; i < numLeds; ++i)
        {
            float y = pos.y + vuHeight - (i + 1) * (ledSize.y + ledSpacing);

            if (i < activeLeds)
            {
                ImU32 color = (i == numLeds - 1) ? IM_COL32(255, 0, 0, 255) : (i >= 5) ? IM_COL32(255, 255, 0, 255)
                                                                                       : IM_COL32(0, 255, 0, 255);

                drawList->AddRectFilled(
                    ImVec2(pos.x, y),
                    ImVec2(pos.x + ledSize.x, y + ledSize.y),
                    color, 2.0f);
            }
            else
            {
                drawList->AddRect(
                    ImVec2(pos.x, y),
                    ImVec2(pos.x + ledSize.x, y + ledSize.y),
                    IM_COL32(60, 60, 60, 255), 2.0f);
            }
        }
        ImGui::PopID(); // End unique ID for channel group
        ImGui::EndGroup();

        if (ch < NUM_CHANNELS - 1)
            ImGui::SameLine();
    }

    ImGui::EndGroup();
    ImGui::Separator();

    // ListBox for profiles
    std::vector<const char *> items;
    for (const auto &p : profiles)
        items.push_back(p.label.c_str());

    ImGui::Text("Profiles:");

    static bool shouldScrollToSelected = true;

    if (ImGui::BeginListBox("##", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8)))
    {
        for (int i = 0; i < items.size(); ++i)
        {
            bool isSelected = (i == selectedProfile);
            if (ImGui::Selectable(items[i], isSelected))
            {
                selectedProfile = i;
                shouldScrollToSelected = false;
            }

            // Apply profile on double-click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) // 0 = left mouse button
            {
                applyProfile(profiles[i], p_exit);
            }

            // Auto-scroll
            if (isSelected && shouldScrollToSelected && !ImGui::IsItemVisible())
            {
                ImGui::SetScrollHereY(0.5f);
                shouldScrollToSelected = false; // Reset after scrolling
            }
        }
        ImGui::EndListBox();
    }

    if (ImGui::Button("Apply"))
    {
        if (selectedProfile >= 0 && selectedProfile < (int)profiles.size())
        {
            applyProfile(profiles[selectedProfile], p_exit);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Editor"))
    {
        launchEditor();
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit"))
    {
        if (p_exit)
            *p_exit = true;
    }

    ImGui::End();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;

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
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2,
                                      D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"EQSwitchImGui", nullptr};
    ::RegisterClassExW(&wc);
    // HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"EQ Switch", WS_OVERLAPPEDWINDOW, 100, 100, (int)(800 * main_scale), (int)(600 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    const wchar_t *windowTitle = L"EQ Switch";

    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName,
        windowTitle, // <-- MUST be a wide string literal or pointer
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, (int)(800 * main_scale), (int)(600 * main_scale),
        nullptr, nullptr, wc.hInstance, nullptr);

    OutputDebugStringW(L"Creating window with title EQ Switch...\n");

    ::SetWindowTextW(hwnd, windowTitle);

    // Set window icon (small and big)
    HICON hIcon = (HICON)LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon)
    {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    // SetWindowTextW(hwnd, L"EQ Switch");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "config/settings.ini";
    (void)io;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    style.FontSizeBase = 20.0f;
    ImFont *font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Carlito-Regular.ttf");

    // ImFont *smallFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Carlito-Regular.ttf", 16.0f);
    IM_ASSERT(font != nullptr);
    // IM_ASSERT(smallFont != nullptr);

    g_SmallFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Carlito-Regular.ttf");
    if (!g_SmallFont)
    {
        std::cerr << "Failed to load font!\n";
    }

    std::thread audioThread([]()
                            { CaptureAudioLoop(NUM_CHANNELS); });
    audioThread.detach();

    // Main loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Your EQ Switch window
        ShowEQSwitchWindow(&done);

        // Rendering
        ImGui::Render();
        const float clear_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
