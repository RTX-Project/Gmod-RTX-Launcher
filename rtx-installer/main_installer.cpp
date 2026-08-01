// main_installer.cpp - Изолированный C++ инсталлятор RTX Launcher (ImGui + DirectX 11)
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <d3d11.h>

#include "../rtx-launcher/imgui/imgui.h"
#include "../rtx-launcher/imgui/backends/imgui_impl_win32.h"
#include "../rtx-launcher/imgui/backends/imgui_impl_dx11.h"
#include "../rtx-launcher/HttpClient.h"
#include "../rtx-launcher/LauncherUpdater.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

namespace fs = std::filesystem;

// DirectX 11 глобальные переменные
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

static bool CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Состояние установки
enum class InstallState { Idle, Downloading, CreatingShortcut, Complete, Error };
static InstallState g_installState = InstallState::Idle;
static std::atomic<float> g_downloadProgress{ 0.0f };
static float g_downloadProgressSmooth = 0.0f;
static std::wstring g_statusText = L"Нажмите «Начать установку» для продолжения.";
static std::wstring g_errorMsg = L"";
static std::mutex g_statusMutex;
static float g_autoLaunchTimer = 2.5f;
static bool g_autoLaunchTriggered = false;

static fs::path GetTargetInstallDir() {
    wchar_t localAppData[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    return fs::path(localAppData) / L"RTXLauncher";
}

// Создание ярлыка на Рабочем столе через COM (IShellLink)
static bool CreateDesktopShortcut(const fs::path& targetExe, const std::wstring& shortcutName) {
    HRESULT hr = CoInitialize(NULL);
    bool success = false;
    IShellLink* pShellLink = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink))) {
        pShellLink->SetPath(targetExe.c_str());
        pShellLink->SetWorkingDirectory(targetExe.parent_path().c_str());
        pShellLink->SetDescription(L"Garry's Mod RTX Remix Launcher");

        IPersistFile* pPersistFile = nullptr;
        if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile))) {
            wchar_t desktopPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
                fs::path shortcutPath = fs::path(desktopPath) / (shortcutName + L".lnk");
                if (SUCCEEDED(pPersistFile->Save(shortcutPath.c_str(), TRUE))) {
                    success = true;
                }
            }
            pPersistFile->Release();
        }
        pShellLink->Release();
    }
    CoUninitialize();
    return success;
}

static void StartInstallationAsync() {
    g_installState = InstallState::Downloading;
    g_downloadProgress = 0.0f;
    g_downloadProgressSmooth = 0.0f;
    {
        std::lock_guard<std::mutex> lock(g_statusMutex);
        g_statusText = L"Подключение к GitHub API...";
    }

    std::thread([]() {
        try {
            fs::path installDir = GetTargetInstallDir();
            
            // Закрываем лаунчер без показа консольного окна
            {
                STARTUPINFOW si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
                wchar_t cmd[] = L"taskkill.exe /F /IM rtx-launcher.exe";
                if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, 5000);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            
            fs::create_directories(installDir);
            fs::path targetExe = installDir / L"rtx-launcher.exe";

            LauncherUpdater::UpdateInfo info = LauncherUpdater::CheckForUpdate();
            std::wstring downloadUrl = info.downloadUrl;
            
            // Если GitHub API недоступно (лимит запросов или приватный репозиторий), используем прямую ссылку-запасной вариант
            if (downloadUrl.empty()) {
                downloadUrl = L"https://github.com/RTX-Project/Gmod-RTX-Launcher/releases/download/v0.0.2.3/system_data.bin";
            }

            {
                std::lock_guard<std::mutex> lock(g_statusMutex);
                g_statusText = L"Скачиваем лаунчер...";
            }

            HttpClient::downloadFile(downloadUrl, targetExe.wstring(), L"RTXLauncherInstaller/1.0",
                [](uint64_t downloaded, uint64_t total) -> bool {
                    if (total > 0) {
                        g_downloadProgress = (float)downloaded / (float)total;
                    } else {
                        g_downloadProgress = 0.5f;
                    }
                    return true;
                });

            {
                std::lock_guard<std::mutex> lock(g_statusMutex);
                g_statusText = L"Создание ярлыка на Рабочем столе...";
            }
            g_installState = InstallState::CreatingShortcut;

            CreateDesktopShortcut(targetExe, L"Garry's Mod RTX Remix");

            {
                std::lock_guard<std::mutex> lock(g_statusMutex);
                g_statusText = L"Установка успешно завершена!";
            }
            g_installState = InstallState::Complete;
        }
        catch (const std::exception& ex) {
            std::string errStr = ex.what();
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &errStr[0], (int)errStr.size(), NULL, 0);
            std::wstring wErr(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &errStr[0], (int)errStr.size(), &wErr[0], size_needed);

            std::lock_guard<std::mutex> lock(g_statusMutex);
            g_errorMsg = std::wstring(L"Ошибка: ") + wErr;
            g_statusText = L"Произошла ошибка при установке.";
            g_installState = InstallState::Error;
        }
    }).detach();
}

static void LaunchInstalledAppAndExit(HWND hWnd) {
    fs::path targetExe = GetTargetInstallDir() / L"rtx-launcher.exe";
    if (fs::exists(targetExe)) {
        ShellExecuteW(NULL, L"open", targetExe.c_str(), NULL, targetExe.parent_path().c_str(), SW_SHOW);
    }
    PostQuitMessage(0);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(101));
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance, hIcon, NULL, NULL, NULL, L"RTXInstallerClass", hIcon };
    RegisterClassExW(&wc);

    int windowWidth = 660;
    int windowHeight = 400;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - windowWidth) / 2;
    int posY = (screenH - windowHeight) / 2;

    HWND hWnd = CreateWindowExW(WS_EX_APPWINDOW, wc.lpszClassName, L"Установка RTX Launcher",
        WS_POPUP | WS_SYSMENU, posX, posY, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hWnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr; // Отключаем файл imgui.ini

    // Загрузка стандартного системного шрифта с поддержкой кириллицы
    ImFontConfig font_config;
    font_config.OversampleH = 4;
    font_config.OversampleV = 4;
    font_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 19.0f, &font_config, io.Fonts->GetGlyphRangesCyrillic());

    // Тёмный графический стиль
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowPadding = ImVec2(24, 20);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.06f, 0.07f, 0.09f, 0.98f);
    colors[ImGuiCol_Border]             = ImVec4(0.18f, 0.22f, 0.28f, 0.60f);
    colors[ImGuiCol_Button]             = ImVec4(0.00f, 0.72f, 0.38f, 0.90f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.00f, 0.85f, 0.44f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.00f, 0.60f, 0.30f, 1.00f);
    colors[ImGuiCol_Text]               = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.55f, 0.58f, 0.65f, 1.00f);

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        // Плавная анимация прогресс-бара
        float dt = io.DeltaTime;
        g_downloadProgressSmooth += (g_downloadProgress.load() - g_downloadProgressSmooth) * dt * 10.0f;

        // Авто-запуск при завершении
        if (g_installState == InstallState::Complete) {
            g_autoLaunchTimer -= dt;
            if (g_autoLaunchTimer <= 0.0f && !g_autoLaunchTriggered) {
                g_autoLaunchTriggered = true;
                LaunchInstalledAppAndExit(hWnd);
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("RTXInstallerWindow", nullptr, windowFlags);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        dl->AddRectFilledMultiColor(winPos, ImVec2(winPos.x + windowWidth, winPos.y + windowHeight), 
            IM_COL32(12, 28, 24, 255), IM_COL32(12, 28, 24, 255),
            IM_COL32(8, 12, 16, 255), IM_COL32(8, 12, 16, 255));

        // MacOS-style window controls
        float btnRadius = 6.0f;
        float spacing = 20.0f;
        
        ImVec2 closePos(winPos.x + 20, winPos.y + 20);
        bool closeHov = ImGui::IsMouseHoveringRect(ImVec2(closePos.x - btnRadius, closePos.y - btnRadius), ImVec2(closePos.x + btnRadius, closePos.y + btnRadius));
        dl->AddCircleFilled(closePos, btnRadius, closeHov ? IM_COL32(255, 95, 86, 255) : IM_COL32(255, 95, 86, 200));
        if (closeHov) {
            dl->AddLine(ImVec2(closePos.x - 3, closePos.y - 3), ImVec2(closePos.x + 3, closePos.y + 3), IM_COL32(50, 0, 0, 255), 1.5f);
            dl->AddLine(ImVec2(closePos.x + 3, closePos.y - 3), ImVec2(closePos.x - 3, closePos.y + 3), IM_COL32(50, 0, 0, 255), 1.5f);
        }
        ImGui::SetCursorPos(ImVec2(closePos.x - btnRadius - winPos.x, closePos.y - btnRadius - winPos.y));
        if (ImGui::InvisibleButton("CloseBtn", ImVec2(btnRadius * 2, btnRadius * 2))) PostQuitMessage(0);

        ImVec2 minPos(winPos.x + 20 + spacing, winPos.y + 20);
        bool minHov = ImGui::IsMouseHoveringRect(ImVec2(minPos.x - btnRadius, minPos.y - btnRadius), ImVec2(minPos.x + btnRadius, minPos.y + btnRadius));
        dl->AddCircleFilled(minPos, btnRadius, minHov ? IM_COL32(255, 189, 46, 255) : IM_COL32(255, 189, 46, 200));
        if (minHov) {
            dl->AddLine(ImVec2(minPos.x - 3, minPos.y), ImVec2(minPos.x + 3, minPos.y), IM_COL32(50, 50, 0, 255), 1.5f);
        }
        ImGui::SetCursorPos(ImVec2(minPos.x - btnRadius - winPos.x, minPos.y - btnRadius - winPos.y));
        if (ImGui::InvisibleButton("MinBtn", ImVec2(btnRadius * 2, btnRadius * 2))) ShowWindow(hWnd, SW_MINIMIZE);

        // --- Верхний заголовок программы ---
        ImGui::SetCursorPos(ImVec2(70, 12));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
        ImGui::Text("GMOD RTX REMIX LAUNCHER");
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(24, 45));
        ImGui::TextDisabled("УСТАНОВКА И НАСТРОЙКА");

        // --- Главная карточка содержимого ---
        ImVec2 cardPos = ImVec2(24, 85);
        ImVec2 cardSize = ImVec2(612, 280);
        ImVec2 cardScreenPos = ImVec2(winPos.x + cardPos.x, winPos.y + cardPos.y);
        
        dl->AddRectFilled(cardScreenPos, ImVec2(cardScreenPos.x + cardSize.x, cardScreenPos.y + cardSize.y), IM_COL32(24, 28, 36, 120), 12.0f);
        dl->AddRect(cardScreenPos, ImVec2(cardScreenPos.x + cardSize.x, cardScreenPos.y + cardSize.y), IM_COL32(50, 60, 75, 90), 12.0f, 0, 1.0f);

        ImGui::SetCursorPos(cardPos);
        ImGui::BeginChild("InstallerContentCard", cardSize, false, ImGuiWindowFlags_NoScrollbar);

        ImGui::SetCursorPos(ImVec2(20, 20));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Автоматическая установка лаунчера");
        ImGui::SetCursorPos(ImVec2(20, 48));
        ImGui::TextWrapped("Лаунчер будет установлен в системный профиль пользователя:");

        ImGui::SetCursorPos(ImVec2(20, 75));
        fs::path targetDir = GetTargetInstallDir();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.04f, 0.05f, 0.07f, 1.0f));
        std::string dirUtf8 = targetDir.string();
        ImGui::InputText("##InstallPath", &dirUtf8[0], dirUtf8.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();

        // Полоса прогресса
        ImGui::SetCursorPos(ImVec2(20, 130));
        std::wstring currentStatus;
        {
            std::lock_guard<std::mutex> lock(g_statusMutex);
            currentStatus = g_statusText;
        }

        std::string statusUtf8;
        if (!currentStatus.empty()) {
            int len = WideCharToMultiByte(CP_UTF8, 0, currentStatus.c_str(), (int)currentStatus.size(), NULL, 0, NULL, NULL);
            statusUtf8.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, currentStatus.c_str(), (int)currentStatus.size(), &statusUtf8[0], len, NULL, NULL);
        }
        ImGui::TextColored(ImVec4(0.00f, 0.85f, 0.44f, 1.0f), "%s", statusUtf8.c_str());

        if (!g_errorMsg.empty()) {
            ImGui::SetCursorPos(ImVec2(20, 155));
            std::string errUtf8;
            int len = WideCharToMultiByte(CP_UTF8, 0, g_errorMsg.c_str(), (int)g_errorMsg.size(), NULL, 0, NULL, NULL);
            errUtf8.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, g_errorMsg.c_str(), (int)g_errorMsg.size(), &errUtf8[0], len, NULL, NULL);
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 572.0f);
            ImGui::TextColored(ImVec4(1.0f, 0.30f, 0.30f, 1.0f), "%s", errUtf8.c_str());
            ImGui::PopTextWrapPos();
        }

        if (g_installState != InstallState::Error) {
            ImGui::SetCursorPos(ImVec2(20, 160));
            float fillPct = g_downloadProgressSmooth;
            if (fillPct < 0.0f) fillPct = 0.0f;
            if (fillPct > 1.0f) fillPct = 1.0f;

            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + 572.0f, p0.y + 12.0f);
            dl->AddRectFilled(p0, p1, IM_COL32(30, 40, 50, 255), 6.0f);
            if (fillPct > 0.01f) {
                ImVec2 p2 = ImVec2(p0.x + 572.0f * fillPct, p1.y);
                
                float t = fillPct;
                float r, g, b;
                if (t < 0.5f) {
                    float localT = t * 2.0f;
                    r = 0.92f;
                    g = 0.25f + 0.55f * localT;
                    b = 0.20f;
                } else {
                    float localT = (t - 0.5f) * 2.0f;
                    r = 0.92f - 0.62f * localT;
                    g = 0.80f + 0.05f * localT;
                    b = 0.20f + 0.19f * localT;
                }
                ImU32 fillCol = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
                
                dl->AddRectFilled(p0, p2, fillCol, 6.0f);
            }
        }

        // --- Кнопка действия ---
        ImGui::SetCursorPos(ImVec2(20, 215));
        
        auto DrawActionButton = [](const char* label, float width, bool disabled) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            bool hovered = ImGui::IsMouseHoveringRect(p, ImVec2(p.x + width, p.y + 44.0f));
            bool active = hovered && ImGui::IsMouseDown(0);
            ImU32 col = disabled ? IM_COL32(80, 80, 80, 255) : (active ? IM_COL32(0, 200, 100, 255) : (hovered ? IM_COL32(0, 255, 150, 255) : IM_COL32(0, 255, 128, 255)));
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + width, p.y + 44.0f), col, 8.0f);
            
            ImVec2 textSize = ImGui::CalcTextSize(label);
            ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + width / 2.0f - textSize.x / 2.0f, p.y + 22.0f - textSize.y / 2.0f), 
                disabled ? IM_COL32(200, 200, 200, 255) : IM_COL32(10, 15, 12, 255), label);
                
            return ImGui::InvisibleButton(label, ImVec2(width, 44.0f)) && !disabled;
        };
        
        if (g_installState == InstallState::Idle || g_installState == InstallState::Error) {
            if (DrawActionButton("НАЧАТЬ УСТАНОВКУ", 220, false)) {
                StartInstallationAsync();
            }
        }
        else if (g_installState == InstallState::Downloading || g_installState == InstallState::CreatingShortcut) {
            DrawActionButton("УСТАНОВКА...", 220, true);
        }
        else if (g_installState == InstallState::Complete) {
            std::string btnText = "ЗАПУСТИТЬ ЛАУНЧЕР (" + std::to_string((int)ceil(g_autoLaunchTimer)) + " сек)";
            if (DrawActionButton(btnText.c_str(), 280, false)) {
                LaunchInstalledAppAndExit(hWnd);
            }
        }

        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        const float clearColor[4] = { 0.06f, 0.07f, 0.09f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hWnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helpers DirectX 11
static bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
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
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

static void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_NCHITTEST: {
        // Поддержка перетаскивания окна за область верхней панели
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        if (pt.y >= 0 && pt.y <= 45 && pt.x > 70) {
            return HTCAPTION;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
