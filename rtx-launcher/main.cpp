// main.cpp - Metrostroi RTX Remix Launcher (Приложение Ð´Ð»Ñ  Windows)
//
// Этот файл - главное Ñ ÐµÑ€Ð´Ñ†Ðµ нашего лаунчера! Он ÑƒÐ¿Ñ€Ð°Ð²Ð»Ñ ÐµÑ‚ Ð²Ñ ÐµÐ¼ Ð¸Ð½Ñ‚ÐµÑ€Ñ„ÐµÐ¹Ñ Ð¾Ð¼
// и логикой работы программы (Ñ ÐºÐ°Ñ‡Ð¸Ð²Ð°Ð½Ð¸Ðµ файлов, выбор режимов и Ð·Ð°Ð¿ÑƒÑ Ðº игры).
// 
// Ð˜Ð½Ñ‚ÐµÑ€Ñ„ÐµÐ¹Ñ  теперь работает на движке ImGui + DirectX 11 (d3d11).
// ÐŸÐ¾Ñ Ñ Ð½ÐµÐ½Ð¸Ðµ Ð´Ð»Ñ  тех, кто не в теме: DirectX 11 (или d3d11) â€” Ñ Ñ‚Ð¾ mÐ¾Ñ‰Ð½Ð°Ñ  Ñ‚ÐµÑ…Ð½Ð¾Ð»Ð¾Ð³Ð¸Ñ 
// от Microsoft, ÐºÐ¾Ñ‚Ð¾Ñ€Ð°Ñ  обычно Ð¸Ñ Ð¿Ð¾Ð»ÑŒÐ·ÑƒÐµÑ‚Ñ Ñ  Ð´Ð»Ñ  Ð¾Ñ‚Ñ€Ð¸Ñ Ð¾Ð²ÐºÐ¸ графики в 3D играх. 
// Мы Ð¸Ñ Ð¿Ð¾Ð»ÑŒÐ·ÑƒÐµm её Ð·Ð´ÐµÑ ÑŒ Ð¿Ñ€Ñ Ðmо в лаунчере, чтобы Ð¸Ð½Ñ‚ÐµÑ€Ñ„ÐµÐ¹Ñ  Ð¾Ñ‚Ñ€Ð¸Ñ Ð¾Ð²Ñ‹Ð²Ð°Ð»Ñ Ñ  через 
// вашу видеокарту! Это Ð¿Ð¾Ð·Ð²Ð¾Ð»Ñ ÐµÑ‚ нам делать плавные ÐºÑ€Ð°Ñ Ð¸Ð²Ñ‹Ðµ анимации 
// (например, ÐºÑ€Ð°Ñ Ð¸Ð²Ð¾Ðµ Ð¿Ð¾Ñ Ð²Ð»ÐµÐ½Ð¸Ðµ и затухание менюшек), Ñ Ñ‚Ð¸Ð»ÑŒÐ½Ñ‹Ðµ кнопки и Ñ Ð¾Ð²Ñ€ÐµÐ¼ÐµÐ½Ð½Ñ‹Ð¹ 
// дизайн, который работает очень плавно и Ð²Ñ‹Ð³Ð»Ñ Ð´Ð¸Ñ‚ как Ð¸Ð½Ñ‚ÐµÑ€Ñ„ÐµÐ¹Ñ  Ð½Ð°Ñ Ñ‚Ð¾Ñ Ñ‰ÐµÐ¹ игры.
// 
// Проект можно легко Ñ ÐºÐ¾Ð mÐ¿Ð¸Ð»Ð¸Ñ€Ð¾Ð²Ð°Ñ‚ÑŒ в Visual Studio, никаких Ñ Ð»Ð¾Ð¶Ð½Ñ‹Ñ… Ð½Ð°Ñ Ñ‚Ñ€Ð¾ÐµÐº
// или дополнительных библиотек качать не нужно, Ð²Ñ Ñ‘ уже Ð²Ñ Ñ‚Ñ€Ð¾ÐµÐ½Ð¾!


#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <algorithm>
#include <dwmapi.h>
#include <winioctl.h>
#include <string>
#include <thread>
#undef max
#include <cmath>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <fstream>

#include "HexPatcher.h"
#include "Theme.h"
#include "UI_Overview.h"
#include "UI_Updates.h"
#include "UI_Settings.h"
#include "UI_RtxMods.h"
#include "UI_Authors.h"
#include "UI_Wizard.h"
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "FileSync.h"
#include "SteamBeta.h"
#include "RtxRemixUpdater.h"
#include "BinaryPatcher.h"
#include "GameFixesUpdater.h"
#include "Resource.h"
#include "ArchiveExtract.h"
#include "ModDBScraper.h"
#include "ZipExtract.h"
#include <future>

#include "LauncherUpdater.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "msimg32.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_VAL = 20;
#define DWMWA_USE_IMMERSIVE_DARK_MODE DWMWA_USE_IMMERSIVE_DARK_MODE_VAL
#endif

namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Идентификаторы кнопок и Ñ Ð»ÐµÐ¼ÐµÐ½Ñ‚Ð¾Ð² (внутренние ID)
// ----------------------------------------------------------------------------
enum {
    ID_NAV_OVERVIEW = 100,
    ID_NAV_UPDATES,
    ID_NAV_BACK,
    ID_NAV_MODE_LIGHTING,
    ID_NAV_MODE_FULL,
#ifdef _DEBUG
    ID_BTN_DEV_TEST,
    ID_NAV_DEV,
#endif

    ID_BTN_SYS_MIN,
    ID_BTN_SYS_CLOSE,

    ID_NAV_SETTINGS,

    ID_BTN_REPAIR,
    ID_BTN_LAUNCH_MODE,

    ID_BTN_REPO_BINARY_PATCHES,
    ID_BTN_REPO_FIXES,
    ID_BTN_PATCH_CULLING,
    ID_BTN_PATCH_LODS,
    ID_BTN_PATCH_SKYBOX,

    ID_BTN_REPO_XENTHIO,
    ID_BTN_REPO_BLUEAMULET,

    ID_EDIT_SOURCE = 200,
    ID_EDIT_DEST,
    ID_BTN_BROWSE_SOURCE,
    ID_BTN_BROWSE_DEST,
    ID_BTN_OPEN_DEST,
    ID_BTN_CHANGE_DISK,
    ID_CHK_VERIFY_HASH,
    ID_CHK_DELETE_REMOVED,
    ID_BTN_SYNC,
    ID_BTN_LAUNCH,

    ID_BTN_APPLY_RTX = 300,
    ID_BTN_GITHUB_RTX,
    ID_BTN_REFRESH_RTX,
    ID_LIST_ITEM_FIRST = 1000,
    ID_LIST_ITEM_LAST = 1020,

    ID_BTN_STOP = 500,
    ID_BTN_PAUSE = 501,
    ID_LOG_EDIT,
};

enum class Page {
    Overview = 0, Updates = 1, Settings = 2
#ifdef _DEBUG
    , Developer = 3
#endif
    , UpdateCheck = 4, InstallerWizard = 5, Authors = 6, RtxMods = 7
};

enum class WizardStep { Welcome = 0, LaunchMode = 1, DriveSelect = 2, Progress = 3, Complete = 4 };
enum class RtxModsView { GamesList = 0, ModsList = 1 };

WizardStep g_wizardStep = WizardStep::Welcome;
WizardStep g_wizardTargetStep = WizardStep::Welcome;
bool g_wizardIsSliding = false;
float g_wizardSlideProgress = 0.0f;

bool g_pageIsSliding = false;
float g_pageSlideProgress = 0.0f;
float g_pageSlideDelayTimer = 0.0f;
Page g_pageTarget = Page::Overview;
Page g_pagePrevious = Page::Overview;

RtxModsView g_rtxModsView = RtxModsView::GamesList;
RtxModsView g_rtxModsPreviousView = RtxModsView::GamesList;
bool g_rtxModsIsSliding = false;
float g_rtxModsSlideProgress = 0.0f;
float g_rtxModsSlideDelayTimer = 0.0f;

enum class SidebarMenu { Main, RtxGames };
SidebarMenu g_sidebarMenu = SidebarMenu::Main;
SidebarMenu g_sidebarMenuPrevious = SidebarMenu::Main;

bool g_autoStartGameAfterInstall = true;

// SwitchMainPage moved below

void GoToWizardStep(WizardStep newStep) {
    if (g_wizardStep == newStep) return;
    g_wizardTargetStep = newStep;
    g_wizardIsSliding = true;
    g_wizardSlideProgress = 0.0f;
}

struct DiskInfo {
    std::wstring path;
    std::wstring name;
    uint64_t totalSpace = 0;
    uint64_t freeSpace = 0;
    bool isSSD = false;
    bool isHDD = false;
};

enum class LaunchModeModalState { Closed, Opening, Open, Closing };
LaunchModeModalState g_launchModalState = LaunchModeModalState::Closed;
float g_launchModalTimer = 0.0f;

void (*g_onLaunchModeSelected)() = nullptr;


enum class DiskModalState { Closed, Opening, Open, Transforming, ProgressBar, Closing };
DiskModalState g_diskModalState = DiskModalState::Closed;
float g_diskModalTimer = 0.0f;
float g_diskModalAlpha = 0.0f;

enum class SidebarAnimState { None, WizardOut_MenuIn, SubMenuTransition };
SidebarAnimState g_sidebarAnimState = SidebarAnimState::None;
float g_sidebarAnimTimer = 0.0f;

struct GpuInfo {
    std::string name;
    UINT vendorId = 0;
    UINT deviceId = 0;
    bool isNvidiaRtx = false;
    bool isAmdOrIntel = false;
    bool isGtxOrOlder = false;
    std::string compatibilityNotice;
};

LauncherUpdater::UpdateInfo g_launcherUpdateInfo;
std::atomic<bool> g_isCheckingLauncherUpdate{ false };

// ----------------------------------------------------------------------------
// Глобальное состояние приложения
// ----------------------------------------------------------------------------
struct AppState {
    HWND hMain = nullptr;
    HWND hLblGameStatus = nullptr;
    std::wstring autoGameSourcePath;
    HWND hDownloadPanel = nullptr;
    GpuInfo gpuInfo;

    // ImGui state
    bool receiveBetaUpdates = false;
    bool showChangelog = false;
    std::wstring changelogText;

    std::vector<std::string> consoleLines;
    std::mutex consoleMutex;
    bool showDownloadPanelImgui = false;


    // Download state
    std::atomic<bool> isDownloading{ false };
    bool isFirstLaunchMode = false;
    std::wstring installRootPath;
    std::wstring githubToken;
    int launchMode = 0; // 0 = unset, 1 = normal, 2 = compatibility
    bool hasLaunchedGame = false;
    bool showDiskModal = false;
    bool showLaunchModeModal = false;
    std::vector<DiskInfo> availableDisks;
    HWND hModalOverlay = nullptr;
    HBITMAP modalBgCapture = nullptr;
    void (*onDiskSelected)() = nullptr;

    float downloadProgress = 0.0f;
    float downloadProgressSmooth = 0.0f;
    std::wstring downloadTitleText;
    std::wstring downloadStatsText;
    std::mutex statsMutex;
    float downloadPanelAlpha = 0.0f;
    float downloadPanelOffsetY = 80.0f;

    HWND hNavOverview = nullptr, hNavUpdates = nullptr, hNavSettings = nullptr;
    HWND hNavBack = nullptr, hNavModeLighting = nullptr, hNavModeFull = nullptr;
    HWND hPageOverview = nullptr, hPageUpdates = nullptr, hPageSettings = nullptr, hPageUpdateCheck = nullptr;
    HWND hBtnRepair = nullptr;
    HWND hBtnLaunchMode = nullptr;
    HWND hBtnRepoBinaryPatches = nullptr;
    HWND hBtnRepoFixes = nullptr;

    std::wstring repoBinaryPatches = L"BlueAmulet/SourceRTXTweaks";
    std::wstring repoFixes = L"Xenthio/garrys-mod-rtx-remixed";

    bool optRepair = true;
#ifdef _DEBUG
    HWND hNavDev = nullptr;
    HWND hPageDev = nullptr;
#endif
    HWND hSidebar = nullptr;
    float navIndicatorY = 110.0f;
    float navIndicatorTargetY = 110.0f;
    float navIndicatorWidthMain = 3.0f;
    float navIndicatorWidthSub = 3.0f;

    HWND hBottomBar = nullptr;
    HWND hBtnSync = nullptr, hBtnLaunch = nullptr, hBtnChangeDisk = nullptr;
    HWND hBtnSysMin = nullptr, hBtnSysClose = nullptr;
    HBRUSH brSidebar = nullptr;

    std::wstring oldInstallRootPath;

    bool inSubMenu = false;
    bool menuIsAnimating = false;
    float menuAnimProgress = 0.0f; // 0 = main menu, 1 = sub menu

    Page currentPage = Page::UpdateCheck;

    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> pauseRequested{ false };
    std::atomic<bool> operationRunning{ false };

    std::mutex lastCheckMutex;
    RtxUpdateCheckResult lastCheckResult;

    std::vector<RtxReleaseInfo> rtxReleases;
    int rtxSelectedIndex = -1;

    HWND hLblRtxBanner = nullptr;
    HWND hLblRtxDetails = nullptr;
    HWND hEditRtxDesc = nullptr;
    HWND hBtnApplyRtx = nullptr;
    HWND hBtnGithubRtx = nullptr;
    HWND hBtnRefreshRtx = nullptr;
    std::vector<HWND> hListItems;
    HWND hBtnCancelDownload = nullptr;
    HWND hBtnPauseDownload = nullptr;

    HBRUSH brBgMain = nullptr, brBgPanel = nullptr, brBgInput = nullptr, brBgConsole = nullptr;
};

AppState g_app;

void SwitchMainPage(Page newPage) {
    if (g_app.currentPage == newPage) return;
    g_pagePrevious = g_app.currentPage;
    g_pageTarget = newPage;
    g_pageIsSliding = true;
    g_pageSlideProgress = 0.0f;
    g_pageSlideDelayTimer = 0.0f;
    g_app.currentPage = newPage; // Update instantly so nav indicators highlight correctly
    if (newPage == Page::RtxMods) {
        g_rtxModsView = RtxModsView::GamesList;
        g_rtxModsPreviousView = RtxModsView::GamesList;
        g_rtxModsIsSliding = false;
        g_rtxModsSlideProgress = 0.0f;
        g_rtxModsSlideDelayTimer = 0.0f;
    }
}

void AppendLog(const std::wstring& line);

std::string WStringToUTF8(const std::wstring& wstr);

static std::string g_changelogHistoryText = "";

static void UpdateChangelogCache(const LauncherUpdater::UpdateInfo& info) {
    if (info.releaseNotes.empty()) return;
    
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    fs::path cachePath = fs::path(exePath).parent_path() / L"launcher_changelog.txt";
    
    std::string currentVersionStr = WStringToUTF8(LauncherUpdater::CURRENT_VERSION);
    std::string cachedVersion;
    int lastBuild = 0;
    std::string historyText;

    if (fs::exists(cachePath)) {
        std::ifstream in(cachePath);
        if (in.is_open()) {
            std::string line;
            if (std::getline(in, line)) cachedVersion = line;
            if (std::getline(in, line)) {
                try { lastBuild = std::stoi(line); } catch(...) {}
            }
            std::stringstream ss;
            ss << in.rdbuf();
            historyText = ss.str();
        }
    }

    if (cachedVersion != currentVersionStr) {
        historyText = "";
        lastBuild = 0;
    }

    if (info.buildNumber > lastBuild) {
        std::string newEntry = u8"● Сборка #" + std::to_string(info.buildNumber) + "\n" + WStringToUTF8(info.releaseNotes) + "\n\n";
        historyText = newEntry + historyText;
        lastBuild = info.buildNumber;
        
        std::ofstream out(cachePath);
        if (out.is_open()) {
            out << currentVersionStr << "\n" << lastBuild << "\n" << historyText;
        }
    } else if (info.buildNumber == lastBuild && info.buildNumber > 0) {
        // Отредактировали существующий релиз на GitHub без смены билда. Обновляем верхнюю запись!
        std::string newEntry = u8"● Сборка #" + std::to_string(info.buildNumber) + "\n" + WStringToUTF8(info.releaseNotes) + "\n\n";
        
        std::string searchToken = u8"● Сборка #";
        size_t nextEntryPos = historyText.find(searchToken, searchToken.length());
        if (nextEntryPos != std::string::npos) {
            historyText = newEntry + historyText.substr(nextEntryPos);
        } else {
            historyText = newEntry;
        }
        
        std::ofstream out(cachePath);
        if (out.is_open()) {
            out << currentVersionStr << "\n" << lastBuild << "\n" << historyText;
        }
    }
    
    if (historyText.empty()) {
        g_changelogHistoryText = WStringToUTF8(info.releaseNotes);
    } else {
        g_changelogHistoryText = historyText;
    }
}

void CheckLauncherUpdatesAsync() {
    if (g_isCheckingLauncherUpdate.exchange(true)) return;
    AppendLog(L"[Updater] Запрос списка релизов с GitHub API (" + LauncherUpdater::REPO_OWNER + L"/" + LauncherUpdater::REPO_NAME + L")...");
    std::thread([]() {
        g_launcherUpdateInfo = LauncherUpdater::CheckForUpdate(g_app.githubToken);
        UpdateChangelogCache(g_launcherUpdateInfo);
        g_isCheckingLauncherUpdate = false;

        if (g_launcherUpdateInfo.hasUpdate) {
            std::wstring msg = L"[Updater] ⭐ Найдено обновление: v" + g_launcherUpdateInfo.version;
            if (g_launcherUpdateInfo.buildNumber > 0) {
                msg += L" (Build #" + std::to_wstring(g_launcherUpdateInfo.buildNumber) + L")";
            }
            if (g_launcherUpdateInfo.releaseId > 0) {
                msg += L" [ID: " + std::to_wstring(g_launcherUpdateInfo.releaseId) + L"]";
            }
            AppendLog(msg);
            if (!g_launcherUpdateInfo.downloadUrl.empty()) {
                AppendLog(L"[Updater] Ссылка для скачивания: " + g_launcherUpdateInfo.downloadUrl);
            }
        }
        else if (!g_launcherUpdateInfo.version.empty()) {
            AppendLog(L"[Updater] Установлена последняя версия лаунчера (GitHub: v" + g_launcherUpdateInfo.version + L", Build #" + std::to_wstring(LauncherUpdater::CURRENT_BUILD_NUMBER) + L").");
        }
        else {
            AppendLog(L"[Updater] Не удалось получить данные о релизах с GitHub (возможно, нет соединения или репозиторий приватный).");
        }
    }).detach();
}
static void ShowLaunchModeModal(void (*callback)() = nullptr) {
    g_onLaunchModeSelected = callback;
    g_app.showLaunchModeModal = true;
    g_launchModalState = LaunchModeModalState::Opening;
    g_launchModalTimer = 0.0f;
}

static const int kSidebarWidth = 75;
static const int kHeaderHeight = 64;

// ----------------------------------------------------------------------------
// Ã â€™Ã‘Â Ã Â¿Ã Â¾Ã Â¼Ã Â¾Ã Â³Ã Â°Ã‘â€šÃ ÂµÃ Â»Ã‘Å’Ã Â½Ã‘â€¹Ã Âµ Ã‘â€žÃ‘Æ’Ã Â½Ã ÂºÃ‘â€ Ã Â¸Ã Â¸ UI
// ----------------------------------------------------------------------------

static std::wstring GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthW(hEdit);
    std::wstring buf(len, L'\0');
    if (len > 0) GetWindowTextW(hEdit, &buf[0], len + 1);
    return buf;
}

// ---- Ã Â¡Ã Â¾Ã‘â€¦Ã‘â‚¬Ã Â°Ã Â½Ã ÂµÃ Â½Ã Â¸Ã Âµ Ã Â¿Ã‘Æ’Ã‘â€šÃ ÂµÃ Â¹ source/dest Ã Â¼Ã ÂµÃ Â¶Ã Â´Ã‘Æ’ Ã Â·Ã Â°Ã Â¿Ã‘Æ’Ã‘Â Ã ÂºÃ Â°Ã Â¼Ã Â¸ (Ã Â¿Ã‘â‚¬Ã Â¾Ã‘Â Ã‘â€šÃ Â¾Ã Â¹ Ã‘â€šÃ ÂµÃ ÂºÃ‘Â Ã‘â€šÃ Â¾Ã Â²Ã‘â€¹Ã Â¹ ----
static fs::path GetTempDir() {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    return fs::path(tempPath) / L"rtx-launcher";
}

// Ã‘â€žÃ Â°Ã Â¹Ã Â» Ã‘â‚¬Ã‘Â Ã Â´Ã Â¾Ã Â¼ Ã‘Â  .exe - без Ã‘â‚¬Ã ÂµÃ ÂµÃ‘Â Ã‘â€šÃ‘â‚¬Ã Â°, без доп. Ã Â·Ã Â°Ã Â²Ã Â¸Ã‘Â Ã Â¸Ã Â¼Ã Â¾Ã‘Â Ã‘â€šÃ ÂµÃ Â¹) ----
// Ã‘â€žÃ Â°Ã Â¹Ã Â» Ã‘â‚¬Ã‘Â Ã Â´Ã Â¾Ã Â¼ Ã‘Â  .exe - без Ã‘â‚¬Ã ÂµÃ ÂµÃ‘Â Ã‘â€šÃ‘â‚¬Ã Â°, без доп. Ã Â·Ã Â°Ã Â²Ã Â¸Ã‘Â Ã Â¸Ã Â¼Ã Â¾Ã‘Â Ã‘â€šÃ ÂµÃ Â¹) ----
static fs::path SettingsFilePath() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    return fs::path(exePath).parent_path() / L"launcher_settings.txt";
}

void SaveSettings() {
    std::wofstream f(SettingsFilePath(), std::ios::trunc);
    if (!f) return;
    if (!g_app.installRootPath.empty()) {
        f << L"installRootPath=" << g_app.installRootPath << L"\n";
    }
    f << L"launchMode=" << g_app.launchMode << L"\n";
    f << L"rtxSelectedIndex=" << g_app.rtxSelectedIndex << L"\n";
    if (!g_app.githubToken.empty()) {
        f << L"githubToken=" << g_app.githubToken << L"\n";
    }
    f << L"hasLaunchedGame=" << (g_app.hasLaunchedGame ? 1 : 0) << L"\n";
    f << L"receiveBetaUpdates=" << (g_app.receiveBetaUpdates ? 1 : 0) << L"\n";
}

static void LoadSettings() {
    std::wifstream f(SettingsFilePath());
    if (!f) return;
    std::wstring line;
    while (std::getline(f, line)) {
        if (line.rfind(L"installRootPath=", 0) == 0) {
            g_app.installRootPath = line.substr(16);
        }
        else if (line.rfind(L"launchMode=", 0) == 0) {
            g_app.launchMode = _wtoi(line.substr(11).c_str());
        }
        else if (line.rfind(L"rtxSelectedIndex=", 0) == 0) {
            g_app.rtxSelectedIndex = _wtoi(line.substr(17).c_str());
        }
        else if (line.rfind(L"githubToken=", 0) == 0) {
            g_app.githubToken = line.substr(12);
        }
        else if (line.rfind(L"hasLaunchedGame=", 0) == 0) {
            g_app.hasLaunchedGame = (_wtoi(line.substr(16).c_str()) != 0);
        }
        else if (line.rfind(L"receiveBetaUpdates=", 0) == 0) {
            g_app.receiveBetaUpdates = (_wtoi(line.substr(19).c_str()) != 0);
        }
    }

    LauncherUpdater::INCLUDE_PRERELEASES = g_app.receiveBetaUpdates;

    if (!g_app.installRootPath.empty() && g_app.hBtnChangeDisk) {
        ShowWindow(g_app.hBtnChangeDisk, SW_SHOW);
    }
}

static void SetStatus(const std::wstring& text) {
    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = text; }
}

std::string WStringToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static std::wstring UTF8ToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

static GpuInfo DetectGPU() {
    GpuInfo info;
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        IDXGIAdapter* pAdapter = nullptr;
        if (SUCCEEDED(pFactory->EnumAdapters(0, &pAdapter))) {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
                info.name = WStringToUTF8(desc.Description);
                info.vendorId = desc.VendorId;
                info.deviceId = desc.DeviceId;

                std::string upperName = info.name;
                for (auto& c : upperName) c = (char)toupper((unsigned char)c);

                if (desc.VendorId == 0x10DE) { // NVIDIA
                    if (upperName.find("RTX") != std::string::npos) {
                        info.isNvidiaRtx = true;
                    } else {
                        info.isGtxOrOlder = true;
                        info.compatibilityNotice = u8"Видеокарта NVIDIA GTX: технология RTX Remix задействует аппаратные RT-ядра. На видеокартах без серии RTX стабильность и скорость рендеринга не гарантируются.";
                    }
                }
                else if (desc.VendorId == 0x1002 || desc.VendorId == 0x8086) { // AMD (0x1002) / Intel (0x8086)
                    info.isAmdOrIntel = true;
                    info.compatibilityNotice = u8"Внимание: обнаружена видеокарта AMD Radeon / Intel Arc. Технология NVIDIA RTX Remix создана под архитектуру RTX — производительность и корректность трассировки не гарантируются.";
                }
                else {
                    info.compatibilityNotice = u8"Совместимость графического адаптера с модулем трассировки RTX Remix не гарантируется.";
                }
            }
            pAdapter->Release();
        }
        pFactory->Release();
    }
    return info;
}

void AppendLog(const std::wstring& line) {
    std::lock_guard<std::mutex> lock(g_app.consoleMutex);
    std::string s = WStringToUTF8(line);
    g_app.consoleLines.push_back(s);
    OutputDebugStringW((line + L"\r\n").c_str());
}

static std::wstring BrowseForFolder(HWND owner, const wchar_t* title) {
    std::wstring result;
    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(hr)) {
        DWORD options = 0;
        dialog->GetOptions(&options);
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        dialog->SetTitle(title);
        hr = dialog->Show(owner);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dialog->GetResult(&item))) {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    result = path;
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }
        dialog->Release();
    }
    return result;
}

static void SetControlsEnabled(bool enabled) {
    EnableWindow(g_app.hBtnSync, enabled);
    EnableWindow(g_app.hBtnLaunch, enabled);
    EnableWindow(g_app.hBtnApplyRtx, enabled);
    EnableWindow(g_app.hBtnGithubRtx, enabled);
    EnableWindow(g_app.hBtnRefreshRtx, enabled);
    for (auto h : g_app.hListItems) EnableWindow(h, enabled);
    InvalidateRect(g_app.hBtnSync, nullptr, FALSE);
    InvalidateRect(g_app.hBtnLaunch, nullptr, FALSE);
    InvalidateRect(g_app.hBtnApplyRtx, nullptr, FALSE);
    InvalidateRect(g_app.hBtnGithubRtx, nullptr, FALSE);
    InvalidateRect(g_app.hBtnRefreshRtx, nullptr, FALSE);
    for (auto h : g_app.hListItems) InvalidateRect(h, nullptr, FALSE);
}

static void ShowPage(Page page) {
    if (page == Page::Updates && g_app.currentPage != Page::Updates) {
        if (g_app.rtxReleases.empty()) {
            PostMessageW(g_app.hMain, WM_COMMAND, ID_BTN_REFRESH_RTX, 0);
        }
    }
    g_app.currentPage = page;
    ShowWindow(g_app.hSidebar, page == Page::UpdateCheck ? SW_HIDE : SW_SHOW);
    ShowWindow(g_app.hPageOverview, page == Page::Overview ? SW_SHOW : SW_HIDE);
    ShowWindow(g_app.hPageUpdates, page == Page::Updates ? SW_SHOW : SW_HIDE);
    ShowWindow(g_app.hPageSettings, page == Page::Settings ? SW_SHOW : SW_HIDE);
    if (g_app.hPageUpdateCheck) ShowWindow(g_app.hPageUpdateCheck, page == Page::UpdateCheck ? SW_SHOW : SW_HIDE);
#ifdef _DEBUG
    ShowWindow(g_app.hPageDev, page == Page::Developer ? SW_SHOW : SW_HIDE);
#endif

    if (page == Page::Overview) g_app.navIndicatorTargetY = 110.0f;
    else if (page == Page::Updates) g_app.navIndicatorTargetY = 156.0f;
    else if (page == Page::Settings) g_app.navIndicatorTargetY = 202.0f;
#ifdef _DEBUG
    else if (page == Page::Developer) g_app.navIndicatorTargetY = 248.0f;
#endif
    SetTimer(g_app.hSidebar, 6, 16, nullptr);

    // Force full redraw to clean up artifacts from transparent hidden panels
    InvalidateRect(g_app.hMain, nullptr, TRUE);

    if (g_app.hNavOverview) InvalidateRect(g_app.hNavOverview, nullptr, FALSE);
    if (g_app.hNavUpdates) InvalidateRect(g_app.hNavUpdates, nullptr, FALSE);
    if (g_app.hNavSettings) InvalidateRect(g_app.hNavSettings, nullptr, FALSE);
#ifdef _DEBUG
    if (g_app.hNavDev) InvalidateRect(g_app.hNavDev, nullptr, FALSE);
#endif
}

// ----------------------------------------------------------------------------
// Фоновые операции
// ----------------------------------------------------------------------------

void RunInBackground(std::function<void()> fn) {
    if (g_app.operationRunning.load()) {
        AppendLog(L"Ã Å¸Ã Â¾Ã Â¶Ã Â°Ã Â»Ã‘Æ’Ã Â¹Ã‘Â Ã‘â€šÃ Â° Ã Â´Ã Â¾Ã Â¶Ã Â´Ã Â¸Ã‘â€šÃ ÂµÃ‘Â Ã‘Å’ Ã Â·Ã Â°Ã Â²Ã ÂµÃ‘â‚¬Ã‘Ë†Ã ÂµÃ Â½Ã Â¸Ã‘Â  Ã‘â€šÃ ÂµÃ ÂºÃ‘Æ’Ã‘â€°Ã ÂµÃ Â¹ Ã Â¾Ã Â¿Ã ÂµÃ‘â‚¬Ã Â°Ã‘â€ Ã Â¸Ã Â¸ (или Ã Â½Ã Â°Ã Â¶Ã Â¼Ã Â¸Ã‘â€šÃ Âµ Ã‚Â«Ã Â¡Ã‘â€šÃ Â¾Ã Â¿Ã‚Â»).");
        return;
    }
    g_app.stopRequested = false;
    g_app.operationRunning = true;
    SetControlsEnabled(false);
    std::thread([fn]() {
        try {
            fn();
        }
        catch (const std::exception& e) {
            std::string msg = e.what();
            AppendLog(L"[Ã Å¡Ã Â Ã ËœÃ Â¢Ã ËœÃ Â§Ã â€¢Ã Â¡Ã Å¡Ã Â Ã Â¯ Ã Å¾Ã Â¨Ã ËœÃ â€˜Ã Å¡Ã Â ] " + std::wstring(msg.begin(), msg.end()));
        }
        catch (...) {
            AppendLog(L"[Ã Å¡Ã Â Ã ËœÃ Â¢Ã ËœÃ Â§Ã â€¢Ã Â¡Ã Å¡Ã Â Ã Â¯ Ã Å¾Ã Â¨Ã ËœÃ â€˜Ã Å¡Ã Â ] Ã Â Ã ÂµÃ Â¸Ã Â·Ã Â²Ã ÂµÃ‘Â Ã‘â€šÃ Â½Ã Â¾Ã Âµ Ã Â¸Ã‘Â Ã ÂºÃ Â»Ã‘Å’Ã‘â€¡Ã ÂµÃ Â½Ã Â¸Ã Âµ.");
        }
        g_app.operationRunning = false;
        SetControlsEnabled(true);
        }).detach();
}

static void HideDownloadPanelFade();

void ShowDiskSelectionModal(void (*callback)() = nullptr);

static std::wstring FormatSizeStr(double mb);

static void DoMoveGame(std::wstring src, std::wstring dst) {
    RunInBackground([src, dst]() {
        AppendLog(L"======================================================================");
        AppendLog(L"Ã Å¸Ã ÂµÃ‘â‚¬Ã ÂµÃ Â½Ã Â¾Ã‘Â  папки Ã Â¸Ã Â³Ã‘â‚¬Ã‘â€¹: " + src + L"  ->  " + dst);

        g_app.isDownloading = true;
        g_app.stopRequested = false;
        g_app.pauseRequested = false;
        g_app.downloadProgress = 0.0f;
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        PostMessageW(g_app.hMain, WM_APP + 1, 0, 0);

        std::error_code ec;
        uintmax_t totalBytes = 0;
        if (fs::exists(src, ec)) {
            for (auto& p : fs::recursive_directory_iterator(src, ec)) {
                if (p.is_regular_file(ec)) totalBytes += p.file_size(ec);
            }
        }

        uintmax_t copiedBytes = 0;
        auto startTime = std::chrono::steady_clock::now();
        auto lastUpdate = startTime;
        uintmax_t lastCopied = 0;
        double speed = 0.0;
        int eta = -1;

        std::vector<char> buffer(8 * 1024 * 1024);

        if (fs::exists(src, ec)) {
            for (auto& p : fs::recursive_directory_iterator(src, fs::directory_options::skip_permission_denied, ec)) {
                if (g_app.stopRequested.load()) break;

                if (g_app.pauseRequested.load()) {
                    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                    PostMessageW(g_app.hMain, WM_NULL, 0, 0);
                    while (g_app.pauseRequested.load()) {
                        if (g_app.stopRequested.load()) break;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    startTime = std::chrono::steady_clock::now();
                    lastCopied = copiedBytes;
                    lastUpdate = startTime;
                }

                fs::path relativePath = fs::relative(p.path(), src, ec);
                fs::path targetPath = fs::path(dst) / relativePath;

                if (p.is_directory(ec)) {
                    fs::create_directories(targetPath, ec);
                }
                else if (p.is_regular_file(ec)) {
                    fs::create_directories(targetPath.parent_path(), ec);

                    std::ifstream in(p.path(), std::ios::binary);
                    std::ofstream out(targetPath, std::ios::binary);

                    if (!in || !out) continue;

                    while (in.read(buffer.data(), buffer.size()) || in.gcount() > 0) {
                        if (g_app.stopRequested.load()) break;
                        out.write(buffer.data(), in.gcount());
                        copiedBytes += in.gcount();

                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
                        if (elapsed > 100) {
                            double dt = elapsed / 1000.0;
                            if (dt > 0) {
                                double instantSpeed = ((copiedBytes - lastCopied) / (1024.0 * 1024.0)) / dt;
                                if (speed == 0.0) speed = instantSpeed;
                                else speed = speed * 0.95 + instantSpeed * 0.05;
                            }
                            if (speed > 0) {
                                eta = (int)(((totalBytes - copiedBytes) / (1024.0 * 1024.0)) / speed);
                            }

                            double downMb = copiedBytes / (1024.0 * 1024.0);
                            double totalMb = totalBytes / (1024.0 * 1024.0);

                            wchar_t buf[256];
                            if (eta >= 0) {
                                if (eta <= 59) {
                                    swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с | Осталось около %dс ", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta);
                                }
                                else {
                                    swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с | Осталось около %dм %dс ", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta / 60, eta % 60);
                                }
                            }
                            else {
                                swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с ", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed);
                            }

                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = buf; }
                            if (totalBytes > 0) g_app.downloadProgress = (float)((double)copiedBytes / (double)totalBytes);
                            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);

                            lastCopied = copiedBytes;
                            lastUpdate = now;
                        }
                    }
                }
            }
        }

        if (g_app.stopRequested.load()) {
            AppendLog(L"Перенос отменен!");
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
            g_app.downloadProgress = 1.0f;
            g_app.downloadProgressSmooth = 1.0f;
            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
            g_app.isDownloading = false;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            HideDownloadPanelFade();

            g_app.installRootPath = src;
            SaveSettings();
            return;
        }

        AppendLog(L"Удаление старой папки...");
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
        fs::remove_all(src, ec);

        AppendLog(L"Перенос успешно завершен!");
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        g_app.downloadProgress = 1.0f;
        g_app.downloadProgressSmooth = 1.0f;
        InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);

        g_app.isDownloading = false;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        HideDownloadPanelFade();
        });
}

void OnDiskChanged() {
    if (g_app.installRootPath != g_app.oldInstallRootPath && !g_app.oldInstallRootPath.empty()) {
        DoMoveGame(g_app.oldInstallRootPath, g_app.installRootPath);
    }
}

static std::wstring ShowFolderBrowserDialog() {
    std::wstring result;
    IFileDialog* pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        }
        if (SUCCEEDED(pfd->Show(g_app.hMain))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR pszPath;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    result = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    return result;
}

static void DoSync() {
    std::wstring src = g_app.autoGameSourcePath;
    if (g_app.installRootPath.empty()) {
        ShowDiskSelectionModal(DoSync);
        return;
    }
    std::wstring dst = g_app.installRootPath;
    if (src.empty()) {
        AppendLog(L"Steam не смог найти папку игры. Укажите папку Garry's Mod вручную.");
        src = ShowFolderBrowserDialog();
        if (src.empty()) {
            AppendLog(L"Выбор папки отменен.");
            return;
        }
        g_app.autoGameSourcePath = src;
    }
    std::error_code ec;
    if (!fs::is_directory(src, ec)) {
        AppendLog(L"Исходная папка не найдена: " + src);
        return;
    }

    RunInBackground([src, dst]() {
        AppendLog(L"======================================================================");
        AppendLog(L"Синхронизация: " + src + L"  ->  " + dst);

        g_app.isDownloading = true;
        g_app.downloadProgress = 0.0f;
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        PostMessageW(g_app.hMain, WM_APP + 1, 0, 0);

        FileSync sync;
        sync.verifyHash = false;
        sync.deleteRemoved = true;
        auto stats = sync.sync(fs::path(src), fs::path(dst), GetTempDir() / L"launcher",
            [](const std::wstring& msg) { AppendLog(msg); },
            []() { return g_app.stopRequested.load(); },
            [](float p, const std::wstring& text) {
                g_app.downloadProgress = p;
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = text; }
            },
            []() {
                if (g_app.pauseRequested.load()) {
                    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                    PostMessageW(g_app.hMain, WM_NULL, 0, 0);
                    while (g_app.pauseRequested.load()) {
                        if (g_app.stopRequested.load()) break;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }

            });
        AppendLog(L"ИТОГО: новых " + std::to_wstring(stats.copied) +
            L", обновлено " + std::to_wstring(stats.updated) +
            L", без изменений " + std::to_wstring(stats.skipped) +
            L", удалено " + std::to_wstring(stats.deleted) +
            L", ошибок " + std::to_wstring(stats.errors));

        if (!g_app.stopRequested.load()) {
            AppendLog(L"======================================================================");
            AppendLog(L"Установка автоматических фиксов...");

            // Фиксы временно отключены (будут добавлены пользователем)

            AppendLog(L"Установка фиксов завершена.");

            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
            g_app.downloadProgress = 1.0f;
            g_app.downloadProgressSmooth = 1.0f;
            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);

            g_app.isDownloading = false;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        HideDownloadPanelFade();
        });
}

static void AutoDetectGmod() {
    RunInBackground([]() {
        AppendLog(L"Поиск Garry's Mod в Steam...");
        SteamBeta beta;
        auto result = beta.check();

        std::wstring statusText;
        if (!result.steamFound) {
            statusText = L"Ошибка: Steam не установлен или не найден в реестре.";
            AppendLog(statusText);
        }
        else if (!result.manifestFound) {
            statusText = L"Ошибка: манифест Garry's Mod не найден (игра не установлена).";
            AppendLog(statusText);
        }
        else if (result.installDir.empty() || result.gamePath.empty()) {
            statusText = L"Ошибка: не удалось определить путь установки из манифеста.";
            AppendLog(statusText);
        }
        else {
            g_app.autoGameSourcePath = result.gamePath;
            statusText = L"✓ Garry's Mod найден: " + result.gamePath;
            AppendLog(statusText);
        }

        SetWindowTextW(g_app.hLblGameStatus, statusText.c_str());
        InvalidateRect(g_app.hPageOverview, nullptr, TRUE);
        });
}

static void UpdateRtxUI() {
    int selected = -1;
    {
        std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
        selected = g_app.rtxSelectedIndex;
    }

    if (selected == 0) {
        ShowWindow(g_app.hLblRtxDetails, SW_SHOW);
        SetWindowTextW(g_app.hLblRtxBanner, L"Полное освещение");
        SetWindowTextW(g_app.hLblRtxDetails, L"Режим: Полное освещение\r\n\r\nВ этом режиме оригинальные материалы остаются без изменений, добавляется только рейтрейсинг.");
        SetWindowTextW(g_app.hEditRtxDesc, L"Этот режим отлично подходит для сохранения классического вида игры, но с современными технологиями лучей.");
        ShowWindow(g_app.hBtnApplyRtx, SW_SHOW);
        ShowWindow(g_app.hEditRtxDesc, SW_SHOW);
    }
    else if (selected == 1) {
        ShowWindow(g_app.hLblRtxDetails, SW_SHOW);
        SetWindowTextW(g_app.hLblRtxBanner, L"Освещение + текстуры");
        SetWindowTextW(g_app.hLblRtxDetails, L"Режим: Освещение и текстуры\r\n\r\nВ этом режиме будут загружены улучшенные PBR текстуры для игры.");
        SetWindowTextW(g_app.hEditRtxDesc, L"Максимальное качество графики, переработанные материалы для RTX Remix.");
        ShowWindow(g_app.hBtnApplyRtx, SW_SHOW);
        ShowWindow(g_app.hEditRtxDesc, SW_SHOW);
    }
    else {
        SetWindowTextW(g_app.hLblRtxBanner, L"Выберите режим слева");
        SetWindowTextW(g_app.hLblRtxDetails, L"");
        SetWindowTextW(g_app.hEditRtxDesc, L"");
        ShowWindow(g_app.hBtnApplyRtx, SW_HIDE);
        ShowWindow(g_app.hEditRtxDesc, SW_HIDE);
        ShowWindow(g_app.hLblRtxDetails, SW_HIDE); // Прячем и текст, чтобы не было чёрного прямоугольника
    }

    // Принудительно перерисовываем родительскую панель, чтобы стереть старый текст (проблема прозрачного фона)
    InvalidateRect(g_app.hPageUpdates, nullptr, TRUE);
}

void LoadRtxReleases() {
    PostMessageW(g_app.hMain, WM_COMMAND, ID_BTN_REFRESH_RTX, 1);
}

// Forward declaration - defined after DoApplyRtxVersion
static bool CheckSteamBranch();

static std::wstring FormatSizeStr(double mb) {
    wchar_t buf[64];
    if (mb >= 1000.0) {
        swprintf_s(buf, L"%.1f ГБ", mb / 1024.0);
    }
    else {
        swprintf_s(buf, L"%.1f МБ", mb);
    }
    return buf;
}

// Helper function to extract embedded zip to a target directory
static bool ExtractResourceZipToDir(int resourceId, const std::wstring& destDir) {
    HRSRC hRes = FindResourceW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(resourceId), (LPCWSTR)RT_RCDATA);
    if (!hRes) return false;
    HGLOBAL hData = LoadResource(GetModuleHandleW(nullptr), hRes);
    if (!hData) return false;
    DWORD size = SizeofResource(GetModuleHandleW(nullptr), hRes);
    void* data = LockResource(hData);
    if (!data) return false;

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring tempZip = std::wstring(tempPath) + L"dxvk_temp_res.7z";

    FILE* f = nullptr;
    if (_wfopen_s(&f, tempZip.c_str(), L"wb") != 0 || !f) return false;
    fwrite(data, 1, size, f);
    fclose(f);

    bool success = true;
    try {
        ArchiveExtract::extractAndFlatten(tempZip, destDir);
    }
    catch (...) {
        success = false;
    }

    DeleteFileW(tempZip.c_str());
    return success;
}

static void DoApplyRtxVersion() {
    std::wstring workingPath = g_app.installRootPath;
    if (workingPath.empty()) {
        ShowDiskSelectionModal(DoApplyRtxVersion);
        return;
    }

    std::wstring srcPath = g_app.autoGameSourcePath;
    if (srcPath.empty()) {
        AppendLog(L"[Steam] Путь к оригинальной игре не найден. Перейдите на вкладку Обзор и дождитесь инициализации.");
        return;
    }

    RunInBackground([srcPath, workingPath]() {
        AppendLog(L"====================================================================");
        AppendLog(L"Начало установки RTX версии...");

        g_app.isDownloading = true;
        g_app.stopRequested = false;
        g_app.pauseRequested = false;
        g_app.downloadProgress = 0.0f;
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        PostMessageW(g_app.hMain, WM_APP + 1, 0, 0); // Trigger ShowDownloadPanelFade

        AppendLog(L"Копирование оригинальной игры в " + workingPath + L"...");

        FileSync sync;
        sync.verifyHash = false;
        sync.deleteRemoved = true;

        sync.sync(fs::path(srcPath), fs::path(workingPath), GetTempDir() / L"launcher",
            [](const std::wstring& msg) { AppendLog(msg); },
            []() { return g_app.stopRequested.load(); },
            [](float p, const std::wstring& text) {
                g_app.downloadProgress = p;
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = text; }
            },
            []() {
                if (g_app.pauseRequested.load()) {
                    // spin if paused
                }
            });

        if (g_app.stopRequested.load()) {
            g_app.isDownloading = false;
            HideDownloadPanelFade();
            AppendLog(L"Установка отменена пользователем.");
            return;
        }

        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Установка DXVK..."; g_app.downloadStatsText = L""; }
        AppendLog(L"Извлечение встроенных файлов DXVK RTX...");
        bool dxvkOk = ExtractResourceZipToDir(IDR_DXVK_ZIP, workingPath + L"\\bin\\win64");
        if (!dxvkOk) {
            AppendLog(L"Ошибка: не удалось извлечь встроенный ZIP архив dxvk.");
        }

        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Скачивание с GitHub..."; g_app.downloadStatsText = L""; }
        AppendLog(L"Скачивание и применение фиксов...");

        GameFixesUpdater::DownloadAndApplyFixes(workingPath, L"Xenthio/garrys-mod-rtx-remixed",
            [](const std::wstring& msg) { AppendLog(msg); },
            [](float p, const std::wstring& title, const std::wstring& stats) {
                g_app.downloadProgress = p;
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = title; g_app.downloadStatsText = stats; }
            });

        HexPatcher::ApplyPatches(workingPath, [](const std::wstring& msg) { AppendLog(msg); });

        std::wstring topbrPath = workingPath + L"\\rtx-remix\\mods\\~gmod_topbr";
        AppendLog(L"Удаление папки ~gmod_topbr: " + topbrPath);
        std::error_code ec;
        fs::remove_all(topbrPath, ec);

        g_app.downloadProgress = 1.0f;
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
        AppendLog(L"Установка RTX версии успешно завершена.");

        std::this_thread::sleep_for(std::chrono::seconds(2));
        g_app.isDownloading = false;
        HideDownloadPanelFade();
        });
}

void DoOpenRtxGithub() {
    RtxReleaseInfo release;
    {
        std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
        if (g_app.rtxSelectedIndex >= 0 && g_app.rtxSelectedIndex < (int)g_app.rtxReleases.size()) {
            release = g_app.rtxReleases[g_app.rtxSelectedIndex];
        }
    }
    if (!release.htmlUrl.empty()) {
        ShellExecuteW(g_app.hMain, L"open", release.htmlUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

// Проверка бета-ветки Steam (вызывается из фона, возвращает true если всё в порядке)
static bool CheckSteamBranch() {
    SteamBeta beta;
    auto result = beta.check();
    if (!result.steamFound) {
        AppendLog(L"[Steam] Не удалось найти установку Steam. Проверьте вручную.");
        return false;
    }
    if (!result.manifestFound) {
        AppendLog(L"[Steam] Не найден appmanifest Garry's Mod. Убедитесь, что игра установлена.");
        return false;
    }
    if (!result.onExpectedBranch) {
        std::wstring msg = L"[ВНИМАНИЕ] Garry's Mod не на бета-ветке 'x86-64'.";
        if (!result.betaKey.empty())
            msg += L" Текущая бета-ветка: '" + result.betaKey + L"'.";
        else
            msg += L" Убедитесь, что в свойствах игры выбрана правильная бета-версия.";
        msg += L" Рекомендуемая: Steam -> Garry's Mod -> Свойства -> Бета-версии -> x86-64.";
        AppendLog(msg);
        return false;
    }
    AppendLog(L"[Steam] Бета-ветка 'x86-64' активна. ОК.");
    return true;
}

static bool CheckIfSSD(const std::wstring& drivePath) {
    std::wstring volumePath = L"\\\\.\\" + drivePath.substr(0, 2);
    HANDLE hDevice = CreateFileW(volumePath.c_str(), 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) return false;

    STORAGE_PROPERTY_QUERY query = {};
    query.PropertyId = StorageDeviceSeekPenaltyProperty;
    query.QueryType = PropertyStandardQuery;

    DEVICE_SEEK_PENALTY_DESCRIPTOR result = {};
    DWORD bytesReturned = 0;
    bool isSSD = false;

    if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &query, sizeof(query), &result, sizeof(result), &bytesReturned, nullptr)) {
        isSSD = !result.IncursSeekPenalty;
    }
    CloseHandle(hDevice);
    return isSSD;
}

static std::vector<DiskInfo> GetAvailableDisks() {
    std::vector<DiskInfo> disks;
    DWORD driveMask = GetLogicalDrives();
    for (int i = 0; i < 26; ++i) {
        if (driveMask & (1 << i)) {
            wchar_t driveLetter[] = { (wchar_t)(L'A' + i), L':', L'\\', L'\0' };
            UINT type = GetDriveTypeW(driveLetter);
            if (type == DRIVE_FIXED) {
                wchar_t volName[MAX_PATH] = {};
                GetVolumeInformationW(driveLetter, volName, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0);

                ULARGE_INTEGER freeBytes, totalBytes, totalFree;
                GetDiskFreeSpaceExW(driveLetter, &freeBytes, &totalBytes, &totalFree);

                DiskInfo info;
                info.path = driveLetter;
                info.name = volName;
                if (info.name.empty()) info.name = L"Локальный диск";
                info.totalSpace = totalBytes.QuadPart;
                info.freeSpace = freeBytes.QuadPart;
                info.isSSD = CheckIfSSD(driveLetter);
                info.isHDD = !info.isSSD;

                disks.push_back(info);
            }
        }
    }
    return disks;
}

void DoLaunchGame();

void ShowDiskSelectionModal(void (*callback)()) {
    g_app.onDiskSelected = callback;
    g_app.availableDisks = GetAvailableDisks();
    g_diskModalState = DiskModalState::Opening;
    g_diskModalTimer = 0.0f;
}

void DoLaunchGame() {
    if (g_app.launchMode == 0) {
        ShowLaunchModeModal(DoLaunchGame);
        return;
    }
    if (g_app.installRootPath.empty()) {
        ShowDiskSelectionModal(DoLaunchGame);
        return;
    }

    std::wstring srcPath = g_app.autoGameSourcePath;
    std::wstring dstPath = g_app.installRootPath;

    if (srcPath.empty()) {
        AppendLog(L"[Steam] Путь к Garry's Mod не определён. Перейдите на страницу Обзор и дождитесь определения пути.");
        return;
    }

    std::error_code ec;
    if (!fs::is_directory(srcPath, ec)) {
        AppendLog(L"Исходная папка не найдена: " + srcPath);
        return;
    }

    RunInBackground([srcPath, dstPath]() mutable {
        AppendLog(L"====================================================================");
        g_app.isDownloading = true;
        g_app.stopRequested = false;
        g_app.pauseRequested = false;
        g_app.downloadProgress = 0.05f;
        g_app.downloadProgressSmooth = 0.05f;

        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Проверка конфигурации Steam..."; g_app.downloadStatsText = L""; }
        AppendLog(L"Проверка бета-ветки Steam перед запуском...");
        bool branchOk = CheckSteamBranch();

        if (!branchOk) {
            AppendLog(L"Запуск отменён: неправильная бета-ветка.");
            g_app.isDownloading = false;
            return;
        }

        if (dstPath.empty()) {
            dstPath = L"C:\\Metrostroi RTX";
            g_app.installRootPath = dstPath;
            SaveSettings();
        }

        bool hasExe = fs::exists(dstPath + L"\\gmod.exe") || fs::exists(dstPath + L"\\hl2.exe") || fs::exists(dstPath + L"\\bin\\win64\\gmod.exe");
        bool hasRtx = fs::exists(dstPath + L"\\bin\\win64\\d3d9.dll");
        bool isFirstLaunch = (!hasExe || !hasRtx);
        g_app.isFirstLaunchMode = isFirstLaunch;

        if (isFirstLaunch) {
            AppendLog(L"Первый запуск: полная установка компонентов...");
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] Подготовка файлов..."; g_app.downloadStatsText = L""; }

            // 1. Полное копирование файлов из Steam
            {
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] Копирование файлов..."; g_app.downloadStatsText = L""; }
                g_app.downloadProgress = 0.10f;

                FileSync sync;
                sync.verifyHash = false;
                sync.deleteRemoved = true;
                sync.sync(fs::path(srcPath), fs::path(dstPath), GetTempDir() / L"launcher",
                    [](const std::wstring& msg) { AppendLog(msg); },
                    []() { return g_app.stopRequested.load(); },
                    [](float p, const std::wstring& text) {
                        g_app.downloadProgress = 0.10f + p * 0.55f;
                        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = text; }
                    },
                    []() {
                        if (g_app.pauseRequested.load()) {
                        }
                    });

                if (g_app.stopRequested.load()) {
                    g_app.isDownloading = false;
                    HideDownloadPanelFade();
                    AppendLog(L"Запуск отменён пользователем.");
                    return;
                }
            }

            // 2. Распаковка DXVK RTX
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] Распаковка DXVK RTX..."; g_app.downloadStatsText = L""; }
            g_app.downloadProgress = 0.75f;
            AppendLog(L"Извлечение встроенных файлов DXVK RTX...");
            ExtractResourceZipToDir(IDR_DXVK_ZIP, dstPath + L"\\bin\\win64");

            // 3. Применение патчей
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] Применение патчей..."; g_app.downloadStatsText = L""; }
            g_app.downloadProgress = 0.85f;
            AppendLog(L"Применение бинарных патчей...");
            HexPatcher::ApplyPatches(dstPath, [](const std::wstring& msg) { AppendLog(msg); });

            // 4. Фиксы с GitHub
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] Загрузка фиксов..."; g_app.downloadStatsText = L""; }
            g_app.downloadProgress = 0.90f;
            AppendLog(L"Скачивание и применение фиксов...");
            GameFixesUpdater::DownloadAndApplyFixes(dstPath, L"Xenthio/garrys-mod-rtx-remixed",
                [](const std::wstring& msg) { AppendLog(msg); },
                [](float p, const std::wstring& title, const std::wstring& stats) {
                    g_app.downloadProgress = 0.90f + p * 0.08f;
                    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[Установка] " + title; g_app.downloadStatsText = stats; }
                });

            std::wstring topbrPath = dstPath + L"\\rtx-remix\\mods\\~gmod_topbr";
            AppendLog(L"Удаление папки ~gmod_topbr: " + topbrPath);
            std::error_code ec;
            fs::remove_all(topbrPath, ec);
        }
        else {
            AppendLog(L"Быстрый запуск: проверка обновлений и старт...");
            std::wstring prefix = g_autoStartGameAfterInstall ? L"[Запуск] " : L"[Установка] ";
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = prefix + L"Проверка обновлений..."; g_app.downloadStatsText = L""; }
            g_app.downloadProgress = 0.50f;

            if (!fs::exists(dstPath + L"\\bin\\win64\\dxvk.conf")) {
                ExtractResourceZipToDir(IDR_DXVK_ZIP, dstPath + L"\\bin\\win64");
            }

            // Быстрая проверка обновления фиксов с GitHub
            bool autoStart = g_autoStartGameAfterInstall;
            GameFixesUpdater::DownloadAndApplyFixes(dstPath, L"Xenthio/garrys-mod-rtx-remixed",
                [](const std::wstring& msg) { AppendLog(msg); },
                [autoStart](float p, const std::wstring& title, const std::wstring& stats) {
                    g_app.downloadProgress = 0.50f + p * 0.40f;
                    std::wstring pfx = autoStart ? L"[Проверка] " : L"[Установка] ";
                    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = pfx + title; g_app.downloadStatsText = stats; }
                });
        }

        // ----------------------------------------------------------------
        // Загрузка мода с ModDB перед запуском игры
        // ----------------------------------------------------------------
        if (g_app.rtxSelectedIndex == 0 || g_app.rtxSelectedIndex == 1) { // Metrostroi RTX (Light или Full)
            std::wstring modFolderName = (g_app.rtxSelectedIndex == 0) ? L"Metrostroi_light" : L"Metrostroi_full";
            std::wstring targetModPath = dstPath + L"\\rtx-remix\\mods\\" + modFolderName;
            
            if (!fs::exists(targetModPath)) {
                std::error_code ec;
                fs::create_directories(targetModPath, ec);
            }

            std::wstring versionsPath = targetModPath + L"\\versions.txt";
            std::wstring currentVersionStr = L"";
            if (fs::exists(versionsPath)) {
                std::wifstream vf(versionsPath);
                std::getline(vf, currentVersionStr);
                vf.close();
            }

            AppendLog(L"Проверка актуальности мода " + modFolderName + L"...");
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[ModDB] Проверка версии " + modFolderName + L"..."; g_app.downloadStatsText = L"Получение ссылки..."; }
            
            std::promise<std::wstring> urlPromise;
            // У ModDB разные ссылки для Light и Full? Для примера используем один и тот же проект
            ModDBScraper::FetchLatestDownloadUrlAsync(L"https://www.moddb.com/mods/metrostroi-rtx", 
                [&urlPromise](std::wstring url) { urlPromise.set_value(url); },
                [&urlPromise]() { urlPromise.set_value(L""); }
            );
            
            std::wstring downloadUrl = urlPromise.get_future().get();
            if (downloadUrl.empty()) {
                AppendLog(L"Ошибка: не удалось получить ссылку с ModDB.");
                g_app.isDownloading = false;
                return;
            }

            if (downloadUrl == currentVersionStr) {
                AppendLog(L"Установлена актуальная версия мода. Пропуск скачивания.");
            } else {
                AppendLog(L"Найдена новая версия: " + downloadUrl);
                std::wstring zipPath = dstPath + L"\\moddb_temp.zip";
                
                auto startTime = std::chrono::steady_clock::now();
                
                try {
                    HttpClient::downloadFile(downloadUrl, zipPath, L"RTX-Launcher", 
                        [startTime](uint64_t downloaded, uint64_t totalSize) -> bool {
                            if (g_app.stopRequested) return false;
                            while (g_app.pauseRequested && !g_app.stopRequested) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            }
                            
                            float p = totalSize > 0 ? (float)downloaded / (float)totalSize : 0.0f;
                            g_app.downloadProgress = 0.5f + p * 0.48f; 
                            
                            auto now = std::chrono::steady_clock::now();
                            auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
                            
                            std::wstring stats;
                            if (elapsedSec > 0) {
                                double speed = (double)downloaded / (1024.0 * 1024.0 * elapsedSec); // MB/s
                                uint64_t remainingBytes = totalSize > downloaded ? totalSize - downloaded : 0;
                                double speedBytes = (double)downloaded / (double)elapsedSec;
                                double etaSec = speedBytes > 0 ? (double)remainingBytes / speedBytes : 0;
                                
                                int etaMin = (int)etaSec / 60;
                                int etaS = (int)etaSec % 60;
                                
                                wchar_t buf[256];
                                swprintf(buf, 256, L"%.1f МБ / %.1f МБ | Скорость: %.1f МБ/с | Осталось: %d мин %d сек",
                                    (double)downloaded / (1024.0*1024.0), (double)totalSize / (1024.0*1024.0),
                                    speed, etaMin, etaS);
                                stats = buf;
                            } else {
                                stats = L"Подключение...";
                            }
                            
                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[ModDB] Скачивание " + modFolderName + L"..."; g_app.downloadStatsText = stats; }
                            
                            return true;
                        });
                        
                    if (g_app.stopRequested) {
                        fs::remove(zipPath);
                        g_app.isDownloading = false;
                        return;
                    }
                    
                    { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"[ModDB] Распаковка архива..."; g_app.downloadStatsText = L"Это может занять несколько минут..."; }
                    AppendLog(L"Распаковка " + zipPath + L" в " + targetModPath + L"...");
                    ZipExtract::extractAll(zipPath, targetModPath);
                    fs::remove(zipPath);
                    
                    // Обновляем versions.txt
                    std::wofstream vf(versionsPath, std::ios::trunc);
                    vf << downloadUrl;
                    vf.close();
                    
                    AppendLog(L"Мод успешно скачан и установлен!");
                } catch (const std::exception& e) {
                    AppendLog(L"Ошибка скачивания/распаковки ModDB: " + UTF8ToWString(e.what()));
                    g_app.isDownloading = false;
                    return;
                }
            }
        }

        if (!g_autoStartGameAfterInstall) {
            g_app.isDownloading = false;
            g_app.downloadProgress = 0.0f;
            HideDownloadPanelFade();
            GoToWizardStep(WizardStep::Complete);
            return;
        }

        // Запуск игры
        g_app.downloadProgress = 1.0f;
        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Запуск Garry's Mod..."; g_app.downloadStatsText = L"Игра запускается"; }

        std::wstring exePath;
        for (auto& name : { std::wstring(L"gmod.exe"), std::wstring(L"hl2.exe"), std::wstring(L"bin\\win64\\gmod.exe"), std::wstring(L"bin\\win64\\hl2.exe") }) {
            std::wstring candidate = dstPath + L"\\" + name;
            if (fs::exists(candidate)) { exePath = candidate; break; }
        }

        if (exePath.empty()) {
            AppendLog(L"gmod.exe не найден в папке игры. Ошибка конфигурации.");
            g_app.isDownloading = false;
            return;
        }

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        std::wstring launchArgs = L"-dxlevel 90 +mat_disable_d3d9ex 1 -nod3d9ex -windowed -noborder -w " + std::to_wstring(screenWidth) + L" -h " + std::to_wstring(screenHeight);
        if (g_app.launchMode == 2) {
            launchArgs += L" -high +mat_dxlevel 95";
        }
        AppendLog(L"Запуск: " + exePath + L" " + launchArgs);

        SHELLEXECUTEINFOW sei = {};
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"open";
        sei.lpFile = exePath.c_str();
        sei.lpParameters = launchArgs.c_str();
        sei.lpDirectory = dstPath.c_str();
        sei.nShow = SW_SHOWNORMAL;
        if (ShellExecuteExW(&sei)) {
            AppendLog(L"Игра успешно запущена.");
            g_app.hasLaunchedGame = true;
            SaveSettings();
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
            ::ShowWindow(g_app.hMain, SW_MINIMIZE);
        }
        else {
            AppendLog(L"Ошибка при запуске игры.");
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Ошибка запуска!"; }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        g_app.isDownloading = false;
        g_app.downloadProgress = 0.0f;
        {
            std::lock_guard<std::mutex> lock(g_app.statsMutex);
            g_app.downloadTitleText = L"";
            g_app.downloadStatsText = L"";
        }
        HideDownloadPanelFade();
        });
}

static void DoStop() {
    g_app.stopRequested = true;
    g_app.pauseRequested = false;
    AppendLog(L"Ã â€”Ã Â°Ã Â¿Ã‘â‚¬Ã Â¾Ã‘Ë†Ã ÂµÃ Â½Ã Â° Ã Â¾Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â¾Ã Â²Ã ÂºÃ Â° Ã‘â€šÃ ÂµÃ ÂºÃ‘Æ’Ã‘â€°Ã ÂµÃ Â¹ Ã Â¾Ã Â¿Ã ÂµÃ‘â‚¬Ã Â°Ã‘â€ Ã Â¸Ã Â¸...");
}

// Ã Å¸Ã Â»Ã Â°Ã Â²Ã Â½Ã Â¾Ã Âµ Ð¿Ð¾Ñ Ð²Ð»ÐµÐ½Ð¸Ðµ панели Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸ (Ã Â°Ã Â»Ã‘Å’Ã‘â€žÃ Â° 0->1.0 за ~180Ð¼Ñ  Ã‘â€¡Ã ÂµÃ‘â‚¬Ã ÂµÃ Â· AlphaBlend)
static void ShowDownloadPanelFade() {
    g_diskModalState = DiskModalState::ProgressBar;
    g_diskModalTimer = 0.4f;
    g_diskModalAlpha = 1.0f;
    g_app.downloadProgress = 0.0f;
    g_app.downloadProgressSmooth = 0.0f;
}

static void HideDownloadPanelFade() {
    if (g_diskModalState == DiskModalState::ProgressBar) {
        g_diskModalState = DiskModalState::Closing;
        g_diskModalTimer = 0.0f;
    }
}

// ----------------------------------------------------------------------------
// Создание Ã‘Â Ã Â»Ã ÂµÃ Â¼Ã ÂµÃ Â½Ã‘â€šÃ Â¾Ã Â² Ã‘Æ’Ã Â¿Ã‘â‚¬Ã Â°Ã Â²Ã Â»Ã ÂµÃ Â½Ã Â¸Ã‘Â 
// ----------------------------------------------------------------------------

static HFONT g_fontUI = nullptr;
static HFONT g_fontHeading = nullptr;
static HFONT g_fontTitle = nullptr;
static HFONT g_fontMono = nullptr;
static HFONT g_fontSmall = nullptr;

static HWND MakeLabel(HWND parent, const wchar_t* text, int x, int y, int w, int hgt, HFONT font, bool muted = false) {
    HWND hLbl = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, hgt, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hLbl, WM_SETFONT, (WPARAM)font, TRUE);
    SetWindowLongPtrW(hLbl, GWLP_USERDATA, muted ? 1 : 0);
    return hLbl;
}

static HWND MakeCheckbox(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id) {
    HWND chk = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(chk, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    return chk;
}

static HWND MakeButton(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id, bool accent, bool isPill = true) {
    HWND btn = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    Theme::EnableHoverTracking(btn, accent);

    if (isPill) {
        HRGN hRgn = CreateRoundRectRgn(0, 0, w + 1, h + 1, h, h);
        SetWindowRgn(btn, hRgn, TRUE);
    }

    return btn;
}

static HWND MakeNavItem(HWND parent, const wchar_t* text, int y, int id) {
    HWND btn = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | BS_OWNERDRAW,
        0, y, kSidebarWidth, 46, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    Theme::EnableHoverTracking(btn, false);
    return btn;
}


static void BuildOverviewPage(HWND parent) {
    int x = 60, y = 80;

    MakeLabel(parent, L"METROSTROI", x, y, 600, 50, g_fontTitle);
    MakeLabel(parent, L"RTX EXTENSIONS", x, y + 50, 600, 50, g_fontTitle);

    y += 160;
    g_app.hBtnLaunch = MakeButton(parent, L"Ã ËœÃ â€œÃ Â Ã Â Ã Â¢Ã Â¬", x, y, 160, 48, ID_BTN_LAUNCH, true);
    MakeButton(parent, L"  Ã Â Ã Â Ã Â¡Ã Â¢Ã Â Ã Å¾Ã â„¢Ã Å¡Ã Ëœ", x + 180, y, 160, 48, ID_NAV_SETTINGS, false);
    MakeButton(parent, L"  ВЕРСИИ RTX", x + 360, y, 180, 48, ID_NAV_UPDATES, false);

    y += 100;
    g_app.hLblGameStatus = MakeLabel(parent, L"Ã Å¸Ã Â¾Ã Â¸Ã‘Â Ã Âº Garry's Mod в Steam...", x, y, 700, 20, g_fontSmall, true);
    y += 30;
    g_app.hBtnSync = MakeButton(parent, L"Синхронизировать", x, y, 200, 38, ID_BTN_SYNC, false);
    MakeButton(parent, L"Открыть папку", x + 220, y, 160, 38, ID_BTN_OPEN_DEST, false);
    g_app.hBtnChangeDisk = MakeButton(parent, L"Ã Â¡Ã Â¼Ã ÂµÃ Â½Ã Â¸Ã‘â€šÃ‘Å’ Ð´Ð¸Ñ Ðº", x + 400, y, 160, 38, ID_BTN_CHANGE_DISK, false);

    if (g_app.installRootPath.empty()) {
        ShowWindow(g_app.hBtnChangeDisk, SW_HIDE);
    }
}

static void BuildUpdateCheckPage(HWND parent) {
    int w = 350;
    int cx = w / 2;
    int y = 100;
    HWND hTitle = MakeLabel(parent, L"Ã Å¸Ã Â Ã Å¾Ã â€™Ã â€¢Ã Â Ã Å¡Ã Â  Ã Å¾Ã â€˜Ã Â Ã Å¾Ã â€™Ã â€ºÃ â€¢Ã Â Ã ËœÃ â„¢", 0, y, w, 30, g_fontHeading, false);
    SendMessageW(hTitle, WM_SETTEXT, 0, (LPARAM)L"Ã Å¸Ã Â Ã Å¾Ã â€™Ã â€¢Ã Â Ã Å¡Ã Â  Ã Å¾Ã â€˜Ã Â Ã Å¾Ã â€™Ã â€ºÃ â€¢Ã Â Ã ËœÃ â„¢");
    HWND hSub = MakeLabel(parent, L"Ã Å¸Ã Â¾Ã Â´Ã Â¾Ã Â¶Ã Â´Ã Â¸Ã‘â€šÃ Âµ, Ã Â¸Ã Â´Ã ÂµÃ‘â€š Ã‘Â Ã Â²Ã‘Â Ã Â·Ã‘Å’ Ã‘Â  Ã‘Â Ã ÂµÃ‘â‚¬Ã Â²Ã ÂµÃ‘â‚¬Ã Â¾Ã Â¼...", 0, y + 40, w, 20, g_fontSmall, true);
    // Center text in static controls
    SetWindowLongPtrW(hTitle, GWL_STYLE, GetWindowLongPtrW(hTitle, GWL_STYLE) | SS_CENTER);
    SetWindowLongPtrW(hSub, GWL_STYLE, GetWindowLongPtrW(hSub, GWL_STYLE) | SS_CENTER);
}

static void BuildSettingsPage(HWND parent) {
    int x = 32, y = 24;
    MakeLabel(parent, L"Ã â€ Ã Å¾Ã Å¸Ã Å¾Ã â€ºÃ Â Ã ËœÃ Â¢Ã â€¢Ã â€ºÃ Â¬Ã Â Ã Â«Ã â€¢ Ã Â Ã Â Ã Â¡Ã Â¢Ã Â Ã Å¾Ã â„¢Ã Å¡Ã Ëœ", x, y, 600, 30, g_fontHeading);
    y += 40;
    MakeLabel(parent, L"Ã â€ Ã Â»Ã‘Â  Ã ÂºÃ Â¾Ã‘â‚¬Ã‘â‚¬Ã ÂµÃ ÂºÃ‘â€šÃ Â½Ã Â¾Ã Â¹ Ã‘â‚¬Ã Â°Ã Â±Ã Â¾Ã‘â€šÃ‘â€¹ мода Ã Â¾Ã Â±Ã‘Â Ã Â·Ã Â°Ã‘â€šÃ ÂµÃ Â»Ã‘Å’Ã Â½Ã Â¾ Ã‘â€šÃ‘â‚¬Ã ÂµÃ Â±Ã‘Æ’Ã ÂµÃ‘â€šÃ‘Â Ã‘Â  Ã Â±Ã ÂµÃ‘â€šÃ Â°-Ã Â²Ã ÂµÃ‘â‚¬Ã‘Â Ã Â¸Ã‘Â  Steam (x86-64).",
        x, y, 600, 20, g_fontSmall, true);

    y += 40;
    MakeLabel(parent, L"Ã ËœÃ Â½Ã‘Â Ã‘â€šÃ‘â‚¬Ã‘Æ’Ã Â¼Ã ÂµÃ Â½Ã‘â€šÃ‘â€¹ Ã Â²Ã Â¾Ã‘Â Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â¾Ã Â²Ã Â»Ã ÂµÃ Â½Ã Â¸Ã‘Â  мода:", x, y, 600, 20, g_fontUI, true);
    y += 26;
    g_app.hBtnRepair = MakeButton(parent, L"Ã Â£Ã Â´Ã Â°Ã Â»Ã‘Â Ã‘â€šÃ‘Å’ Ã Â»Ã Â¸Ã‘Ë†Ã Â½Ã Â¸Ã Âµ Ã‘â€žÃ Â°Ã Â¹Ã Â»Ã‘â€¹ Ã Â¿Ã‘â‚¬Ã Â¸ Ã‘Â Ã Â¸Ã Â½Ã‘â€¦Ã‘â‚¬Ã Â¾Ã Â½Ã Â¸Ã Â·Ã Â°Ã‘â€ Ã Â¸Ã Â¸", x, y, 320, 34, ID_BTN_REPAIR, g_app.optRepair);

    y += 50;
    MakeLabel(parent, L"Режим Ð·Ð°Ð¿ÑƒÑ ÐºÐ°:", x, y, 600, 20, g_fontUI, true);
    y += 26;
    g_app.hBtnLaunchMode = MakeButton(parent, g_app.launchMode == 2 ? L"Режим: Ð¡Ð¾Ð²Ð¼ÐµÑ Ñ‚Ð¸Ð¼Ð¾Ñ Ñ‚ÑŒ" : L"Режим: Обычный", x, y, 320, 34, ID_BTN_LAUNCH_MODE, false);

    y += 50;
    MakeLabel(parent, L"Бинарные патчи:", x, y, 600, 20, g_fontUI, true);
    y += 26;
    g_app.hBtnRepoBinaryPatches = MakeButton(parent, L"Ã ËœÃ‘Â Ã‘â€šÃ Â¾Ã‘â€¡Ã Â½Ã Â¸Ã Âº: BlueAmulet/SourceRTXTweaks", x, y, 360, 34, ID_BTN_REPO_BINARY_PATCHES, false);

    y += 46;
    MakeLabel(parent, L"Ã â€˜Ã Â°Ã Â·Ã Â° Ã Â¸Ã‘Â Ã Â¿Ã‘â‚¬Ã Â°Ã Â²Ã Â»Ã ÂµÃ Â½Ã Â¸Ã Â¹:", x, y, 600, 20, g_fontUI, true);
    y += 26;
    g_app.hBtnRepoFixes = MakeButton(parent, L"Ã ËœÃ‘Â Ã‘â€šÃ Â¾Ã‘â€¡Ã Â½Ã Â¸Ã Âº: Xenthio/garrys-mod-rtx-remixed", x, y, 360, 34, ID_BTN_REPO_FIXES, false);
}


static void BuildUpdatesPage(HWND parent) {
    int rx = 40;

    g_app.hLblRtxBanner = MakeLabel(parent, L"ВЫБЕРИТЕ ВЕРСИЮ", rx, 30, 600, 40, g_fontTitle);

    g_app.hBtnApplyRtx = MakeButton(parent, L"Ã Â£Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â¾Ã Â²Ã Â¸Ã‘â€šÃ‘Å’ Ã Â²Ã ÂµÃ‘â‚¬Ã‘Â Ã Â¸Ã‘Å½", rx, 90, 200, 40, ID_BTN_APPLY_RTX, true);
    ShowWindow(g_app.hBtnApplyRtx, SW_HIDE);

    int contentY = 170;

    g_app.hLblRtxDetails = MakeLabel(parent, L"", rx, contentY, 240, 250, g_fontUI, true);
    ShowWindow(g_app.hLblRtxDetails, SW_HIDE); // Ã Å¸Ã Â¾Ã ÂºÃ Â°Ã Â·Ã‘â€¹Ã Â²Ã Â°Ã ÂµÃ‘â€šÃ‘Â Ã‘Â  Ã‘â€šÃ Â¾Ã Â»Ã‘Å’Ã ÂºÃ Â¾ Ã Â¿Ã‘â‚¬Ã Â¸ Ã Â²Ã‘â€¹Ã Â±Ã Â¾Ã‘â‚¬Ã Âµ Ã Â²Ã ÂµÃ‘â‚¬Ã‘Â Ã Â¸Ã Â¸

    g_app.hEditRtxDesc = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
        rx + 260, contentY, 400, 250, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(g_app.hEditRtxDesc, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    ShowWindow(g_app.hEditRtxDesc, SW_HIDE);
}


#ifdef _DEBUG
static void BuildDevPage(HWND parent) {
    int x = 32, y = 24;
    MakeLabel(parent, L"Ã Å“Ã â€¢Ã Â Ã Â® Ã Â Ã Â Ã â€”Ã Â Ã Â Ã â€˜Ã Å¾Ã Â¢Ã Â§Ã ËœÃ Å¡Ã Â ", x, y, 600, 30, g_fontHeading);
    y += 34;
    MakeLabel(parent, L"Ã Â­Ã‘â€šÃ Â° Ã‘Â Ã‘â€šÃ‘â‚¬Ã Â°Ã Â½Ã Â¸Ã‘â€ Ã Â° Ã Â´Ã Â¾Ã‘Â Ã‘â€šÃ‘Æ’Ã Â¿Ã Â½Ã Â° Ã‘â€šÃ Â¾Ã Â»Ã‘Å’Ã ÂºÃ Â¾ в Ã Â¾Ã‘â€šÃ Â»Ã Â°Ã Â´Ã Â¾Ã‘â€¡Ã Â½Ã Â¾Ã Â¹ Ã‘Â Ã Â±Ã Â¾Ã‘â‚¬Ã ÂºÃ Âµ (Debug).",
        x, y, 700, 20, g_fontSmall, true);

    y += 44;
    MakeButton(parent, L"Ã Â¢Ã ÂµÃ‘Â Ã‘â€š Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸ UI", x, y, 200, 40, ID_BTN_DEV_TEST, true);
}
#endif

// ----------------------------------------------------------------------------
// Ã Å¾Ã ÂºÃ Â¾Ã Â½Ã Â½Ã Â°Ã‘Â  Ã Â¿Ã‘â‚¬Ã Â¾Ã‘â€ Ã ÂµÃ Â´Ã‘Æ’Ã‘â‚¬Ã Â°
// ----------------------------------------------------------------------------

static LRESULT CALLBACK PanelSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (msg) {
        // Ã Å¸Ã ÂµÃ‘â‚¬Ã ÂµÃ Â½Ã Â°Ã Â¿Ã‘â‚¬Ã Â°Ã Â²Ã Â»Ã‘Â Ã ÂµÃ Â¼ Ã Â²Ã Â°Ã Â¶Ã Â½Ã‘â€¹Ã Âµ Ã‘Â Ã Â¾Ã Â¾Ã Â±Ã‘â€°Ã ÂµÃ Â½Ã Â¸Ã‘Â  Ã Â¾Ã‘â€š Ã Â´Ã Â¾Ã‘â€¡Ã ÂµÃ‘â‚¬Ã Â½Ã Â¸Ã‘â€¦ Ã‘Â Ã Â»Ã ÂµÃ Â¼Ã ÂµÃ Â½Ã‘â€šÃ Â¾Ã Â² (кнопки, Ã‘Â Ã‘â€šÃ Â°Ã‘â€šÃ Â¸Ã ÂºÃ Â¸)
        // в главное окно (WndProc), Ã‘â€¡Ã‘â€šÃ Â¾Ã Â±Ã‘â€¹ Ã Â¾Ã Â±Ã‘â‚¬Ã Â°Ã Â±Ã Â°Ã‘â€šÃ‘â€¹Ã Â²Ã Â°Ã‘â€šÃ‘Å’ Ã Â²Ã‘Â Ã‘Å½ Ã Â»Ã Â¾Ã Â³Ã Â¸Ã ÂºÃ‘Æ’ в одном Ã Â¼Ã ÂµÃ‘Â Ã‘â€šÃ Âµ
    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN:
        return SendMessageW(GetParent(hwnd), msg, wParam, lParam);

    case WM_ERASEBKGND: {
        return 0; // Transparent to allow parent background image to show
    }

    case WM_TIMER: {
        if (wParam == 2 && hwnd == g_app.hSidebar) {
            float step = 0.08f;
            if (g_app.inSubMenu) {
                g_app.menuAnimProgress += step;
                if (g_app.menuAnimProgress >= 1.0f) {
                    g_app.menuAnimProgress = 1.0f;
                    g_app.menuIsAnimating = false;
                    KillTimer(hwnd, 2);
                }
            }
            else {
                g_app.menuAnimProgress -= step;
                if (g_app.menuAnimProgress <= 0.0f) {
                    g_app.menuAnimProgress = 0.0f;
                    g_app.menuIsAnimating = false;
                    KillTimer(hwnd, 2);
                }
            }

            float p = g_app.menuAnimProgress;
            auto smoothStep = [](float a, float b, float x) -> float {
                if (x <= a) return 0.0f;
                if (x >= b) return 1.0f;
                float t = (x - a) / (b - a);
                return t * t * (3.0f - 2.0f * t);
                };

            float widthMainP = smoothStep(0.0f, 0.4f, p);
            float widthSubP = smoothStep(0.6f, 1.0f, p);
            float slideP = smoothStep(0.2f, 0.8f, p);

            g_app.navIndicatorWidthMain = 3.0f + 67.0f * widthMainP;
            g_app.navIndicatorWidthSub = 70.0f - 67.0f * widthSubP;

            Theme::g_accentTransition = slideP;

            int mainX = (int)(-kSidebarWidth * slideP);
            int subX = (int)(kSidebarWidth * (1.0f - slideP));

            RedrawWindow(g_app.hSidebar, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
            if (g_app.hBtnSync) InvalidateRect(g_app.hBtnSync, nullptr, FALSE);
            if (g_app.hBtnLaunch) InvalidateRect(g_app.hBtnLaunch, nullptr, FALSE);
            if (g_app.hBtnApplyRtx) InvalidateRect(g_app.hBtnApplyRtx, nullptr, FALSE);
            if (g_app.hBtnGithubRtx) InvalidateRect(g_app.hBtnGithubRtx, nullptr, FALSE);
            if (g_app.hDownloadPanel && g_app.isDownloading) InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);

#ifdef _DEBUG
            HDWP hdwp = BeginDeferWindowPos(7);
#else
            HDWP hdwp = BeginDeferWindowPos(6);
#endif
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavOverview, nullptr, mainX, 110, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavUpdates, nullptr, mainX, 156, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavSettings, nullptr, mainX, 202, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#ifdef _DEBUG
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavDev, nullptr, mainX, 248, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif

            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavBack, nullptr, subX, 110, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavModeLighting, nullptr, subX, 156, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hdwp) hdwp = DeferWindowPos(hdwp, g_app.hNavModeFull, nullptr, subX, 202, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            if (hdwp) EndDeferWindowPos(hdwp);

            // Invalidate all buttons to prevent bit-blit artifacts during movement
            if (g_app.hNavOverview) InvalidateRect(g_app.hNavOverview, nullptr, FALSE);
            if (g_app.hNavUpdates) InvalidateRect(g_app.hNavUpdates, nullptr, FALSE);
            if (g_app.hNavSettings) InvalidateRect(g_app.hNavSettings, nullptr, FALSE);
            if (g_app.hNavModeLighting) InvalidateRect(g_app.hNavModeLighting, nullptr, FALSE);
            if (g_app.hNavModeFull) InvalidateRect(g_app.hNavModeFull, nullptr, FALSE);
            if (g_app.hNavBack) InvalidateRect(g_app.hNavBack, nullptr, FALSE);
#ifdef _DEBUG
            if (g_app.hNavDev) InvalidateRect(g_app.hNavDev, nullptr, FALSE);
#endif

            return 0;
        }
        else if (wParam == 6 && hwnd == g_app.hSidebar) {
            float target = g_app.navIndicatorTargetY;
            if (g_app.navIndicatorY != target) {
                float diff = target - g_app.navIndicatorY;
                if (abs(diff) < 0.5f) {
                    g_app.navIndicatorY = target;
                    KillTimer(hwnd, 6);
                }
                else {
                    g_app.navIndicatorY += diff * 0.25f;
                }
                if (g_app.hNavOverview) InvalidateRect(g_app.hNavOverview, nullptr, FALSE);
                if (g_app.hNavUpdates) InvalidateRect(g_app.hNavUpdates, nullptr, FALSE);
                if (g_app.hNavSettings) InvalidateRect(g_app.hNavSettings, nullptr, FALSE);
                if (g_app.hNavModeLighting) InvalidateRect(g_app.hNavModeLighting, nullptr, FALSE);
                if (g_app.hNavModeFull) InvalidateRect(g_app.hNavModeFull, nullptr, FALSE);
#ifdef _DEBUG
                if (g_app.hNavDev) InvalidateRect(g_app.hNavDev, nullptr, FALSE);
#endif
            }
            else {
                KillTimer(hwnd, 6);
            }
            return 0;
        }
        break;
    }

                 // Ã Â¡Ã Â¾Ã Â±Ã‘Â Ã‘â€šÃ Â²Ã ÂµÃ Â½Ã Â½Ã Â°Ã‘Â  Ã Â¾Ã‘â€šÃ‘â‚¬Ã Â¸Ã‘Â Ã Â¾Ã Â²Ã ÂºÃ Â° Ã‘ÂžÃ‘â€žÃ Â¾Ã Â½Ã Â° Ð´Ð»Ñ  Ã‘Â Ã‘â€šÃ‘â‚¬Ã Â°Ã Â½Ã Â¸Ã‘â€ Ã‘â€¹ обновлений и Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã Â¾Ã‘â€¡Ã Â½Ã Â¾Ã Â¹ панели
    case WM_PAINT: {
        if (hwnd == g_app.hPageUpdateCheck) { // UpdateCheckPage modal card
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc; GetClientRect(hwnd, &rc);
            Gdiplus::Graphics g(hdc);
            g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            Gdiplus::GraphicsPath path;
            Gdiplus::RectF r((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
            Theme::RoundedRectPath(path, r, 12.0f);
            Gdiplus::SolidBrush br(Gdiplus::Color(240, 20, 20, 22)); // dark card
            g.FillPath(&br, &path);
            Gdiplus::Pen pen(Gdiplus::Color(100, 255, 255, 255), 1.0f);
            g.DrawPath(&pen, &path);
            EndPaint(hwnd, &ps);
            return 0;
        }
        else if (hwnd == g_app.hPageUpdates) {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        else if (hwnd == g_app.hDownloadPanel) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // ÃƒÂ Ã¢â‚¬â„¢Ãƒâ€˜Ã¢â€šÂ¬Ãƒâ€˜Ã†â€™Ãƒâ€˜Ã¢â‚¬Â¡ÃƒÂ Ã‚Â½Ãƒâ€˜Ã†â€™Ãƒâ€˜Ã…Â½ ÃƒÂ Ã‚Â¸Ãƒâ€˜Ã‚Â ÃƒÂ Ã‚ÂºÃƒÂ Ã‚Â»Ãƒâ€˜Ã…Â½Ãƒâ€˜Ã¢â‚¬Â¡ÃƒÂ Ã‚Â°ÃƒÂ Ã‚Â¸ÃƒÂ Ã‚Â¼ ÃƒÂ Ã‚Â´ÃƒÂ Ã‚Â¾Ãƒâ€˜Ã¢â‚¬Â¡ÃƒÂ Ã‚ÂµÃƒâ€˜Ã¢â€šÂ¬ÃƒÂ Ã‚Â½ÃƒÂ Ã‚Â¸ÃƒÂ Ã‚Âµ ÃƒÂ Ã‚Â¾ÃƒÂ Ã‚ÂºÃƒÂ Ã‚Â½ÃƒÂ Ã‚Â° (ÃƒÂ Ã‚ÂºÃƒÂ Ã‚Â½ÃƒÂ Ã‚Â¾ÃƒÂ Ã‚Â¿ÃƒÂ Ã‚ÂºÃƒÂ Ã‚Â¸) ÃƒÂ Ã‚Â¸ÃƒÂ Ã‚Â· HDC
            HWND hChild = GetWindow(hwnd, GW_CHILD);
            while (hChild) {
                if (IsWindowVisible(hChild)) {
                    RECT childRc;
                    GetWindowRect(hChild, &childRc);
                    MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&childRc, 2);
                    ExcludeClipRect(hdc, childRc.left, childRc.top, childRc.right, childRc.bottom);
                }
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
            RECT rc; GetClientRect(hwnd, &rc);
            int panW = rc.right - rc.left;
            int panH = rc.bottom - rc.top;

            // Ã Â Ã Â¸Ã‘Â Ã‘Æ’Ã ÂµÃ Â¼ Ã Â¿Ã Â°Ã Â½Ã ÂµÃ Â»Ã‘Å’ в Ã Â¾Ã‘â€žÃ‘â€žÃ‘Â Ã ÂºÃ‘â‚¬Ã Â¸Ã Â½Ã Â½Ã‘â€¹Ã Â¹ bitmap, Ã Â·Ã Â°Ã‘â€šÃ ÂµÃ Â¼ AlphaBlend'им на Ã‘Â Ã ÂºÃ‘â‚¬Ã Â°Ã Â½
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, panW, panH);
            HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

            // Ã‘â€žÃ Â°Ã Â¹Ã Â» Ã‘â‚¬Ã‘Â Ã Â´Ã Â¾Ã Â¼ Ã‘Â  .exe - без Ã‘â‚¬Ã ÂµÃ ÂµÃ‘Â Ã‘â€šÃ‘â‚¬Ã Â°, без доп. Ã Â·Ã Â°Ã Â²Ã Â¸Ã‘Â Ã Â¸Ã Â¼Ã Â¾Ã‘Â Ã‘â€šÃ ÂµÃ Â¹) ----
            {
                Graphics bg(memDC);
                bg.SetSmoothingMode(SmoothingModeAntiAlias);
                // Ã Â¡Ã Â½Ã Â°Ã‘â€¡Ã Â°Ã Â»Ã Â° Ð·Ð°Ð¿Ð¾Ð»Ð½Ñ ÐµÐ¼ Ã Â²Ã ÂµÃ‘Â Ã‘Å’ bitmap Ã‘â€žÃ Â¾Ã Â½Ã Â¾Ã Â¼ Ã‘Â Ã ÂºÃ‘â‚¬Ã Â°Ã Â½Ã Â°
                SolidBrush bgFill(Theme::kBgMain); // Ð¾Ñ Ð½Ð¾Ð²Ð½Ð¾Ð¹ Ã‘â€ Ã Â²Ã ÂµÃ‘â€š Ã‘Â Ã ÂºÃ‘â‚¬Ã Â°Ã Â½Ã Â°
                bg.FillRectangle(&bgFill, 0, 0, panW, panH);

                SolidBrush panelBrush(Theme::kBgPanel);
                RectF panelRc(0.5f, 0.5f, (REAL)panW - 1.0f, (REAL)panH - 1.0f);
                GraphicsPath panelPath;
                Theme::RoundedRectPath(panelPath, panelRc, 8.0f);
                bg.FillPath(&panelBrush, &panelPath);
                Pen borderPen(Theme::kBorder, 1.0f);
                bg.DrawPath(&borderPen, &panelPath);
            }

            // 2. Ã Â¢Ã ÂµÃ ÂºÃ‘Â Ã‘â€š + Ð¿Ð¾Ð»Ð¾Ñ Ð° Ã Â¿Ã‘â‚¬Ã Â¾Ã Â³Ã‘â‚¬Ã ÂµÃ‘Â Ã‘Â Ã Â°
            {
                Graphics g(memDC);
                g.SetSmoothingMode(SmoothingModeAntiAlias);
                g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

                float barY = (REAL)panH - 18.0f;
                RectF r(16.0f, barY, (REAL)panW - 232.0f, 6.0f); // Ã Å¾Ã‘Â Ã‘â€šÃ Â°Ã Â²Ã Â»Ã‘Â Ã ÂµÃ Â¼ Ã Â¼Ã ÂµÃÂ Ã‘â€šÃ Â¾ Ð´Ð»Ñ  кнопок "Ã Å¾Ã‘â€šÃ Â¼Ã ÂµÃ Â½Ã Â¸Ã‘â€šÃ‘Å’" и "Ã Å¸Ã Â°Ã‘Æ’Ã Â·Ã Â°"

                Font font(Theme::g_fontFamily, 10.0f, FontStyleRegular, UnitPoint);
                SolidBrush textBrush(Theme::kTextPrimary);
                StringFormat format;
                format.SetAlignment(StringAlignmentCenter);
                format.SetLineAlignment(StringAlignmentCenter);
                RectF textRect(r.X, 10.0f, r.Width, barY - 12.0f);

                std::wstring stats;
                {
                    std::lock_guard<std::mutex> lock(g_app.statsMutex);
                    stats = g_app.downloadStatsText;
                }
                if (stats.empty()) stats = L"Ã Å¸Ã Â¾Ã Â´Ã Â³Ã Â¾Ã‘â€šÃ Â¾Ã Â²Ã ÂºÃ Â° к Ã‘Æ’Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â¾Ã Â²Ã ÂºÃ Âµ...";
                g.DrawString(stats.c_str(), -1, &font, textRect, &format, &textBrush);

                GraphicsPath bgPath;
                Theme::RoundedRectPath(bgPath, r, 3.0f);
                SolidBrush bgBrush(Color(255, 55, 55, 65));
                g.FillPath(&bgBrush, &bgPath);

                float prog = g_app.downloadProgressSmooth;
                if (prog > 0.0f) {
                    if (prog > 1.0f) prog = 1.0f;
                    RectF fillR(r.X, r.Y, r.Width * prog, r.Height);
                    if (fillR.Width > 6.0f) {
                        GraphicsPath fillPath;
                        Theme::RoundedRectPath(fillPath, fillR, 3.0f);
                        SolidBrush fillBrush(Theme::CurrentAccent());
                        g.FillPath(&fillBrush, &fillPath);
                    }
                }
            }

            // 3. Ã Â¡Ã Â½Ã Â°Ã‘â€¡Ã Â°Ã Â»Ã Â° Ã‘â‚¬Ã Â¸Ã‘Â Ã‘Æ’Ã ÂµÃ Â¼ Ã‘â€žÃ Â¾Ã Â½ Ã‘â‚¬Ã Â¾Ã Â´Ã Â¸Ã‘â€šÃ ÂµÃ Â»Ã‘Å’Ã‘Â Ã ÂºÃ Â¾Ã Â³Ã Â¾ окна под Ã Â¿Ã Â°Ã Â½Ã ÂµÃ Â»Ã‘Å’, Ã‘â€¡Ã‘â€šÃ Â¾Ã Â±Ã‘â€¹ AlphaBlend Ã‘â‚¬Ã Â°Ã Â±Ã Â¾Ã‘â€šÃ Â°Ã Â» Ã ÂºÃ Â¾Ã‘â‚¬Ã‘â‚¬Ã ÂµÃ ÂºÃ‘â€šÃ Â½Ã Â¾
            FillRect(hdc, &rc, g_app.brBgMain);

            // 4. Ã Â Ã Â°Ã ÂºÃ Â»Ã Â°Ã Â´Ã‘â€¹Ã Â²Ã Â°Ã ÂµÃ Â¼ bitmap Ã‘Â  Ã‘â€šÃ ÂµÃ ÂºÃ‘Æ’Ã‘â€°Ã ÂµÃ Â¹ Ã Â°Ã Â»Ã‘Å’Ã‘â€žÃ Â¾Ã Â¹
            BYTE alpha = (BYTE)(g_app.downloadPanelAlpha * 255.0f);
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, alpha, 0 };
            AlphaBlend(hdc, 0, 0, panW, panH, memDC, 0, 0, panW, panH, bf);

            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
    }
    }
    // Ã â€ Ã Â»Ã‘Â  Ã Â²Ã‘Â Ã ÂµÃ‘â€¦ Ã Â¾Ã‘Â Ã‘â€šÃ Â°Ã Â»Ã‘Å’Ã Â½Ã‘â€¹Ã‘â€¦ Ã‘Â Ã Â¾Ã Â¾Ã Â±Ã‘â€°Ã ÂµÃ Â½Ã Â¸Ã Â¹ Ã Â²Ã‘â€¹Ã Â·Ã‘â€¹Ã Â²Ã Â°Ã ÂµÃ Â¼ Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â´Ã Â°Ã‘â‚¬Ã‘â€šÃ Â½Ã‘â€¹Ã Â¹ Ã Â¾Ã Â±Ã‘â‚¬Ã Â°Ã Â±Ã Â¾Ã‘â€šÃ‘â€¡Ã Â¸Ã Âº
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_MOVE: {
        if (g_app.hModalOverlay) {
            RECT rcMain;
            GetWindowRect(hwnd, &rcMain);
            SetWindowPos(g_app.hModalOverlay, nullptr, rcMain.left, rcMain.top + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    case WM_CREATE: {
        // Ã Â£Ã Â±Ã Â¸Ã‘â‚¬Ã Â°Ã ÂµÃ Â¼ Ã Â¿Ã‘Æ’Ã Â½Ã ÂºÃ‘â€šÃ Â¸Ã‘â‚¬Ã Â½Ã‘â€¹Ã Âµ Ã‘â‚¬Ã Â°Ã Â¼Ã ÂºÃ Â¸ Ã‘â€žÃ Â¾Ã ÂºÃ‘Æ’Ã‘Â Ã Â° Ã‘Â  Ã‘Â Ã Â»Ã ÂµÃ Â¼Ã ÂµÃ Â½Ã‘â€šÃ Â¾Ã Â² (Ã Â¿Ã‘â‚¬Ã Â¸ Ã Â½Ã Â°Ã Â¶Ã Â°Ã‘â€šÃ Â¸Ã Â¸ Alt/Tab)
        SendMessageW(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);

        g_app.brBgMain = CreateSolidBrush(RGB(18, 19, 26));
        g_app.brBgPanel = CreateSolidBrush(RGB(24, 25, 34));
        g_app.brBgInput = CreateSolidBrush(RGB(30, 31, 42));
        g_app.brBgConsole = CreateSolidBrush(RGB(10, 11, 16));

        g_fontUI = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI");
        g_fontHeading = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI");
        g_fontTitle = CreateFontW(-32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI");
        g_fontSmall = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI");
        g_fontMono = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            FIXED_PITCH, L"Cascadia Mono");

        // ---- Сайдбар ----
        RECT initRc; GetClientRect(hwnd, &initRc);
        int initH = initRc.bottom - initRc.top;
        if (initH < 400) initH = 680;

        g_app.brSidebar = CreateSolidBrush(RGB(13, 14, 20));
        g_app.hSidebar = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
            0, 0, kSidebarWidth, 2000, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        HWND hSidebar = g_app.hSidebar;

        HWND hLogo = MakeLabel(hSidebar, L"\xE7F4", 0, 28, kSidebarWidth, 28, g_fontHeading);
        SendMessageW(hLogo, WM_SETFONT, (WPARAM)g_fontHeading, TRUE);

        g_app.hNavOverview = MakeNavItem(hSidebar, L"\xE80F", 110, ID_NAV_OVERVIEW);
        g_app.hNavUpdates = MakeNavItem(hSidebar, L"\xE81C", 156, ID_NAV_UPDATES);
        g_app.hNavSettings = MakeNavItem(hSidebar, L"\xE713", 202, ID_NAV_SETTINGS);
#ifdef _DEBUG
        g_app.hNavDev = MakeNavItem(hSidebar, L"\xE756", 248, ID_NAV_DEV);
#endif

        g_app.hNavBack = MakeNavItem(hSidebar, L"\xE72B", 110, ID_NAV_BACK);
        g_app.hNavModeLighting = MakeNavItem(hSidebar, L"\xE706", 156, ID_NAV_MODE_LIGHTING);
        g_app.hNavModeFull = MakeNavItem(hSidebar, L"\xE790", 202, ID_NAV_MODE_FULL);

        SetWindowPos(g_app.hNavBack, nullptr, kSidebarWidth, 110, kSidebarWidth, 46, SWP_NOZORDER);
        SetWindowPos(g_app.hNavModeLighting, nullptr, kSidebarWidth, 156, kSidebarWidth, 46, SWP_NOZORDER);
        SetWindowPos(g_app.hNavModeFull, nullptr, kSidebarWidth, 202, kSidebarWidth, 46, SWP_NOZORDER);

        // ---- Ã Å¾Ã‘â€šÃ‘â‚¬Ã Â¸Ã‘Â Ã Â¾Ã Â²Ã Â°Ã‘Â  Ã‘Â Ã‘â€šÃ‘â‚¬Ã Â°Ã Â½Ã Â¸Ã‘â€ Ã‘â€¹ Ã Â¼Ã ÂµÃ Â½Ã‘Å½ ----
        g_app.hPageUpdateCheck = CreateWindowW(L"STATIC", L"", WS_CHILD,
            0, 0, 2000, 2000, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        g_app.hPageOverview = CreateWindowW(L"STATIC", L"", WS_CHILD,
            kSidebarWidth, kHeaderHeight, 2000, 600, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        g_app.hPageUpdates = CreateWindowW(L"STATIC", L"", WS_CHILD,
            kSidebarWidth, kHeaderHeight, 2000, 600, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        g_app.hPageSettings = CreateWindowW(L"STATIC", L"", WS_CHILD,
            kSidebarWidth, kHeaderHeight, 2000, 600, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
#ifdef _DEBUG
        g_app.hPageDev = CreateWindowW(L"STATIC", L"", WS_CHILD,
            kSidebarWidth, kHeaderHeight, 2000, 600, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
#endif

        // ---- Ã Â¡Ã Â¸Ã‘Â Ã‘â€šÃ ÂµÃ Â¼Ã Â½Ã‘â€¹Ã Âµ кнопки окна ----
        g_app.hBtnSysMin = MakeButton(hwnd, L"\u2500", 0, 0, 40, 40, ID_BTN_SYS_MIN, false, false);
        g_app.hBtnSysClose = MakeButton(hwnd, L"\u2715", 0, 0, 40, 40, ID_BTN_SYS_CLOSE, false, false);

        SetWindowSubclass(g_app.hSidebar, PanelSubclassProc, 1, 0);
        SetWindowSubclass(g_app.hPageOverview, PanelSubclassProc, 2, 0);
        SetWindowSubclass(g_app.hPageUpdates, PanelSubclassProc, 3, 0);
        SetWindowSubclass(g_app.hPageSettings, PanelSubclassProc, 4, 0);
        SetWindowSubclass(g_app.hBottomBar, PanelSubclassProc, 5, 0);
        SetWindowSubclass(g_app.hPageUpdateCheck, PanelSubclassProc, 7, 0);
#ifdef _DEBUG
        SetWindowSubclass(g_app.hPageDev, PanelSubclassProc, 6, 0);
#endif

        BuildUpdateCheckPage(g_app.hPageUpdateCheck);
        BuildOverviewPage(g_app.hPageOverview);
        BuildUpdatesPage(g_app.hPageUpdates);
        BuildSettingsPage(g_app.hPageSettings);
#ifdef _DEBUG
        BuildDevPage(g_app.hPageDev);
#endif

        // ---- Панель загрузки (AlphaBlend) ----
        g_app.hDownloadPanel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD,
            kSidebarWidth + 24, kHeaderHeight + 476, 2000, 72, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        SetWindowSubclass(g_app.hDownloadPanel, PanelSubclassProc, 5, 0);

        g_app.hBtnCancelDownload = MakeButton(g_app.hDownloadPanel, L"Отменить", 0, 0, 100, 32, ID_BTN_STOP, false, false);
        g_app.hBtnPauseDownload = MakeButton(g_app.hDownloadPanel, L"Пауза", 0, 0, 80, 32, ID_BTN_PAUSE, false, false);

        ShowPage(Page::UpdateCheck);
        SetTimer(hwnd, 999, 2500, nullptr); // Timer for switching to Overview
        LoadSettings();
        AppendLog(L"Ã Å¸Ã‘â‚¬Ã Â¾Ã‘â€ Ã ÂµÃ‘Â Ã‘Â  Ã Â¾Ã‘â€šÃ Â¼Ã ÂµÃ Â½Ã ÂµÃ Â½. Ã Â¤Ã Â°Ã Â¹Ã Â»Ã‘â€¹ Ã Â¾Ã‘Â Ã‘â€šÃ Â°Ã Â»Ã Â¸Ã‘Â Ã‘Å’ Ã Â½Ã ÂµÃ‘â€šÃ‘â‚¬Ã Â¾Ã Â½Ã‘Æ’Ã‘â€šÃ‘â€¹Ã Â¼Ã Â¸. Ã Â Ã Â°Ã Â¶Ã Â¼Ã Â¸Ã‘â€šÃ Âµ \"Ã Â£Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â¾Ã Â²Ã Â¸Ã‘â€šÃ‘Å’\" Ð´Ð»Ñ  Ã Â¿Ã ÂµÃ‘â‚¬Ã ÂµÃ Â·Ã Â°Ã Â¿Ã‘Æ’Ã‘Â Ã ÂºÃ Â°.");
        return 0;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        if (g_app.hPageUpdateCheck) {
            int cardW = 350;
            int cardH = 260;
            SetWindowPos(g_app.hPageUpdateCheck, nullptr, (w - cardW) / 2, (h - cardH) / 2, cardW, cardH, SWP_NOZORDER);
        }
        if (g_app.hSidebar) SetWindowPos(g_app.hSidebar, nullptr, 0, 0, kSidebarWidth, h, SWP_NOZORDER);
        int contentW = w - kSidebarWidth - 24;
        if (contentW < 100) contentW = 100;
        SetWindowPos(g_app.hPageOverview, nullptr, kSidebarWidth, kHeaderHeight, contentW, 460, SWP_NOZORDER);
        if (g_app.hPageUpdates) {
            SetWindowPos(g_app.hPageUpdates, nullptr, kSidebarWidth, kHeaderHeight, contentW, 460, SWP_NOZORDER);
            if (g_app.hEditRtxDesc) {
                int descW = contentW - 300;
                if (descW < 200) descW = 200;
                SetWindowPos(g_app.hEditRtxDesc, nullptr, 300, 170, descW, 250, SWP_NOZORDER);
            }
        }
#ifdef _DEBUG
        SetWindowPos(g_app.hPageDev, nullptr, kSidebarWidth, kHeaderHeight, contentW, h - kHeaderHeight - 80, SWP_NOZORDER);
#endif
        if (g_app.hBottomBar) {
            SetWindowPos(g_app.hBottomBar, nullptr, kSidebarWidth, h - 80, contentW + 24, 80, SWP_NOZORDER);
            if (g_app.hBtnLaunch) {
                SetWindowPos(g_app.hBtnLaunch, nullptr, contentW + 24 - 220, 15, 200, 50, SWP_NOZORDER);
            }
        }
        if (g_app.hBtnSysMin) SetWindowPos(g_app.hBtnSysMin, nullptr, w - 80, 0, 40, 30, SWP_NOZORDER);
        if (g_app.hBtnSysClose) SetWindowPos(g_app.hBtnSysClose, nullptr, w - 40, 0, 40, 30, SWP_NOZORDER);

        if (g_app.hDownloadPanel) {
            SetWindowPos(g_app.hDownloadPanel, nullptr, kSidebarWidth + 24, h - 160 + (int)g_app.downloadPanelOffsetY, contentW - 48, 72, SWP_NOZORDER);
            if (g_app.hBtnCancelDownload) {
                SetWindowPos(g_app.hBtnCancelDownload, nullptr, (contentW - 24) - 120, 20, 100, 32, SWP_NOZORDER);
            }
            if (g_app.hBtnPauseDownload) {
                SetWindowPos(g_app.hBtnPauseDownload, nullptr, (contentW - 24) - 210, 20, 80, 32, SWP_NOZORDER);
            }
        }
        return 0;
    }
    case WM_ERASEBKGND: {
        return 1;
    }
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hCtl = (HWND)lParam;

        wchar_t className[256];
        GetClassNameW(hCtl, className, 256);
        if (wcscmp(className, L"ComboBox") == 0) {
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(30, 31, 42));
            SetTextColor(hdc, RGB(235, 235, 240));
            return (INT_PTR)g_app.brBgInput;
        }

        // Ã Å¸Ã Â¾Ã Â»Ã‘Â  ввода Read-Only Ã Â¾Ã Â±Ã‘â‚¬Ã Â°Ã Â±Ã Â°Ã‘â€šÃ‘â€¹Ã Â²Ã Â°Ã‘Å½Ã‘â€šÃ‘Â Ã‘Â  как Ã‘Â Ã‘â€šÃ Â°Ã‘â€šÃ Â¸Ã‘â€¡Ã Â½Ã‘â€¹Ã Â¹ Ã‘â€šÃ ÂµÃ ÂºÃ‘Â Ã‘â€š в WM_CTLCOLORSTATIC.
        // Фон Ã‘â€šÃ ÂµÃ ÂºÃ‘Â Ã‘â€šÃ Â° должен Ã Â±Ã‘â€¹Ã‘â€šÃ‘Å’ Ã Â¿Ã‘â‚¬Ã Â¾Ã Â·Ã‘â‚¬Ã Â°Ã‘â€¡Ã Â½Ã‘â€¹Ã Â¼ (TRANSPARENT), Ã‘â€ Ã Â²Ã ÂµÃ‘â€š Ã‘â€šÃ ÂµÃ ÂºÃ‘Â Ã‘â€šÃ Â° Ã‘Â Ã‘â€šÃ Â°Ã Â½Ã Â´Ã Â°Ã‘â‚¬Ã‘â€šÃ Â½Ã‘â€¹Ã Â¹.
        // Ã Å¸Ã‘â‚¬Ã Â¸ Ã‘Â Ã‘â€šÃ Â¾Ã Â¼ Ã‘‚¬Ã Â¸Ã‘Â Ã‘Æ’Ã ÂµÃ Â¼ Ñ Ð²Ð¾Ð¹ Ã‘â€žÃ Â¾Ã Â½ Ã‘Â Ã Â»Ã ÂµÃ Â¼Ã ÂµÃ Â½Ã‘â€šÃ Â° (Ã‘â€¡Ã‘â€šÃ Â¾Ã Â±Ã‘â€¹ не Ã Â±Ã‘â€¹Ã Â»Ã Â¾ Ã Â±Ã ÂµÃ Â»Ã‘â€¹Ã‘â€¦ Ã ÂºÃ Â²Ã Â°Ã Â´Ã‘â‚¬Ã Â°Ã‘â€šÃ Â¾Ã Â²).
        if (hCtl == g_app.hEditRtxDesc) {
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(18, 19, 26)); // kBgMain
            SetTextColor(hdc, RGB(235, 235, 240));
            return (INT_PTR)g_app.brBgMain;
        }

        // Ã Â¢Ã ÂµÃ ÂºÃ‘Â Ã‘â€šÃ‘â€¹ Ã‘‚¬Ã Â¸Ã‘Â Ã‘Æ’Ã‘½Ã‘â€šÃ‘Â Ã‘Â  Ã Â¿Ã‘â‚¬Ã Â¾Ã Â·Ã‘â‚¬Ã Â°Ã‘â€¡Ã Â½Ã Â¾.
        LONG_PTR marker = GetWindowLongPtrW(hCtl, GWLP_USERDATA);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, marker == 1 ? RGB(148, 150, 165) : RGB(235, 235, 240));

        return (INT_PTR)GetStockObject(NULL_BRUSH);
    }
    case WM_CTLCOLORLISTBOX: {
        HDC hdcList = (HDC)wParam;
        SetTextColor(hdcList, RGB(235, 235, 240));
        SetBkColor(hdcList, RGB(30, 31, 42)); // Match brBgInput
        SetBkMode(hdcList, OPAQUE);
        return (INT_PTR)g_app.brBgInput;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(30, 31, 42));
        SetTextColor(hdc, RGB(230, 230, 235));
        return (INT_PTR)g_app.brBgInput;
    }
    case WM_DRAWITEM: {
        auto dis = (LPDRAWITEMSTRUCT)lParam;
        wchar_t text[256];
        GetWindowTextW(dis->hwndItem, text, 256);
        int id = dis->CtlID;

        HWND hParent = GetParent(dis->hwndItem);
        if ((id == ID_NAV_OVERVIEW || id == ID_NAV_UPDATES || id == ID_NAV_SETTINGS ||
            id == ID_NAV_BACK || id == ID_NAV_MODE_LIGHTING || id == ID_NAV_MODE_FULL
#ifdef _DEBUG
            || id == ID_NAV_DEV
#endif
            ) && hParent == g_app.hSidebar) {
            bool selected = false;
            if (id == ID_NAV_OVERVIEW && g_app.currentPage == Page::Overview) selected = true;
            if (id == ID_NAV_UPDATES && g_app.currentPage == Page::Updates) selected = true;
            if (id == ID_NAV_SETTINGS && g_app.currentPage == Page::Settings) selected = true;
#ifdef _DEBUG
            if (id == ID_NAV_DEV && g_app.currentPage == Page::Developer) selected = true;
#endif

            if (id == ID_NAV_MODE_LIGHTING || id == ID_NAV_MODE_FULL) {
                int selectedIdx = -1;
                {
                    std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
                    selectedIdx = g_app.rtxSelectedIndex;
                }
                if (id == ID_NAV_MODE_LIGHTING && selectedIdx == 0) selected = true;
                if (id == ID_NAV_MODE_FULL && selectedIdx == 1) selected = true;
            }

            RECT wRect; GetWindowRect(dis->hwndItem, &wRect);
            POINT pt = { 0, wRect.top };
            ScreenToClient(g_app.hSidebar, &pt);
            float buttonY = (float)pt.y;
            float localY = g_app.navIndicatorY - buttonY;

            bool isSubMenuItem = (id == ID_NAV_BACK || id == ID_NAV_MODE_LIGHTING || id == ID_NAV_MODE_FULL);
            float currentIndicatorWidth = isSubMenuItem ? g_app.navIndicatorWidthSub : g_app.navIndicatorWidthMain;

            Gdiplus::Graphics g(dis->hDC);
            g.Clear(Theme::kBgSidebar);
            Theme::DrawNavItem(dis->hDC, dis->rcItem, text, selected, Theme::g_buttonStates[dis->hwndItem], localY, currentIndicatorWidth);
        }
        else if (id >= ID_LIST_ITEM_FIRST && id <= ID_LIST_ITEM_LAST) {
            bool selected = false;
            {
                std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
                if (id - ID_LIST_ITEM_FIRST == g_app.rtxSelectedIndex) selected = true;
            }
            const auto& st = Theme::g_buttonStates[dis->hwndItem];

            Gdiplus::Graphics g(dis->hDC);
            g.Clear(Theme::kBgMain);
            Theme::DrawListItem(dis->hDC, dis->rcItem, text, selected, st);
        }
        else if (id == ID_BTN_SYS_MIN || id == ID_BTN_SYS_CLOSE) {
            auto& st = Theme::g_buttonStates[dis->hwndItem];
            // Draw simple flat sys buttons
            Gdiplus::Graphics g(dis->hDC);
            Gdiplus::Color bg = Theme::kBgMain;
            if (st.hovered) bg = id == ID_BTN_SYS_CLOSE ? Gdiplus::Color(255, 232, 17, 35) : Gdiplus::Color(255, 55, 55, 55);
            if (st.pressed) bg = id == ID_BTN_SYS_CLOSE ? Gdiplus::Color(255, 241, 112, 122) : Gdiplus::Color(255, 80, 80, 80);
            Gdiplus::SolidBrush br(bg);
            g.FillRectangle(&br, 0, 0, dis->rcItem.right, dis->rcItem.bottom);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(235, 235, 240));
            SelectObject(dis->hDC, g_fontUI);
            DrawTextW(dis->hDC, text, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        else {
            auto& st = Theme::g_buttonStates[dis->hwndItem];
            bool enabled = IsWindowEnabled(dis->hwndItem);

            Gdiplus::Color bgCol = Theme::kBgMain;
            if (GetParent(dis->hwndItem) == g_app.hPageSettings) bgCol = Theme::kBgPanel;
            else if (GetParent(dis->hwndItem) == g_app.hDownloadPanel) bgCol = Gdiplus::Color(255, 30, 31, 42); // match download panel bg

            Theme::DrawPillButton(dis->hDC, dis->rcItem, text, st, enabled, bgCol);
        }
        return TRUE;
    }
    case WM_APP + 1: {
        // Ã Å“Ã ÂµÃ Â½Ã ÂµÃ Â´Ã Â¶Ã ÂµÃ‘â‚¬ Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸ Ã‘âžÃ Â°Ã Â¹Ã Â»Ã Â¾Ã Â² и обновление Ã‘Â Ã‘â€šÃ Â°Ã‘â€šÃ‘Æ’Ã‘Â Ã Â°.
        ShowDownloadPanelFade();
        return 0;
    }
    case WM_TIMER: {
        if (wParam == 999) {
            KillTimer(hwnd, 999);
            ShowPage(Page::Overview);
            return 0;
        }
        if (wParam == 3) {
            g_app.downloadPanelAlpha += 0.09f;
            g_app.downloadPanelOffsetY -= 7.2f;
            if (g_app.downloadPanelAlpha >= 1.0f) {
                g_app.downloadPanelAlpha = 1.0f;
                g_app.downloadPanelOffsetY = 0.0f;
                KillTimer(hwnd, 3);
            }
            RECT rcClient; GetClientRect(hwnd, &rcClient);            int h = rcClient.bottom; int contentW = rcClient.right - 250;            SetWindowPos(g_app.hDownloadPanel, nullptr, 250 + 24, h - 160 + (int)g_app.downloadPanelOffsetY, contentW - 48, 72, SWP_NOZORDER);            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
        }
        else if (wParam == 4) {
            // Ã Å¸Ã ÂµÃ‘â‚¬Ã ÂµÃ‘â‚¬Ã Â¸Ã‘Â Ã Â¾Ã Â²Ã ÂºÃ Â° Ã‘Â Ã‘â€šÃ Â°Ã‘â€šÃ‘Æ’Ã‘Â Ã Â° Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸ Ã Â¿Ã‘â‚¬Ã Â¸ изменении
            if (g_app.isDownloading.load()) {
                float target = g_app.downloadProgress;
                if (g_app.downloadProgressSmooth != target) {
                    float diff = target - g_app.downloadProgressSmooth;
                    if (abs(diff) < 0.001f) g_app.downloadProgressSmooth = target;
                    else g_app.downloadProgressSmooth += diff * 0.15f;
                    InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
                }
            }
            else {
                KillTimer(hwnd, 4);
            }
        }
        else if (wParam == 5) {
            // Ã Å¾Ã‘â€šÃ ÂºÃ Â»Ã‘Å½Ã‘â€¡Ã ÂµÃ Â½Ã Â¸Ã Âµ кнопки Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸ ÐµÑ Ð»Ð¸ папка Ã Â½Ã ÂµÃ ÂºÃ Â¾Ã‘â‚¬Ã‘â‚¬Ã ÂµÃ ÂºÃ‘â€šÃ Â½Ã Â°.
            g_app.downloadPanelAlpha -= 0.09f;
            g_app.downloadPanelOffsetY += 7.2f;
            if (g_app.downloadPanelAlpha <= 0.0f) {
                g_app.downloadPanelAlpha = 0.0f;
                g_app.downloadPanelOffsetY = 80.0f;
                KillTimer(hwnd, 5);
                ShowWindow(g_app.hDownloadPanel, SW_HIDE);
            }
            RECT rcClient; GetClientRect(hwnd, &rcClient);
            int h = rcClient.bottom;
            int contentW = rcClient.right - 250; // kSidebarWidth
            SetWindowPos(g_app.hDownloadPanel, nullptr, 250 + 24, h - 160 + (int)g_app.downloadPanelOffsetY, contentW - 48, 72, SWP_NOZORDER);
            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
        }
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
        case ID_NAV_OVERVIEW: ShowPage(Page::Overview); break;
        case ID_NAV_SETTINGS: ShowPage(Page::Settings); break;
        case ID_NAV_UPDATES: {
            ShowPage(Page::Updates);
            if (!g_app.inSubMenu) {
                g_app.inSubMenu = true;
                g_app.menuIsAnimating = true;
                g_app.navIndicatorY = g_app.navIndicatorTargetY;
                SetTimer(g_app.hSidebar, 2, 16, nullptr);
            }
            break;
        }
#ifdef _DEBUG
        case ID_NAV_DEV: ShowPage(Page::Developer); break;
        case ID_BTN_DEV_TEST:
            if (!g_app.isDownloading.exchange(true)) {
                g_app.stopRequested = false;
                g_app.pauseRequested = false;
                g_app.downloadProgress = 0.0f;
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                ShowDownloadPanelFade();

                RunInBackground([]() {
                    uint64_t totalBytes = 1024ULL * 1024ULL * 1024ULL; // 1 GB
                    uint64_t downloadedBytes = 0;

                    auto startTime = std::chrono::steady_clock::now();
                    auto lastUpdate = startTime;
                    double pausedSeconds = 0.0;

                    while (downloadedBytes < totalBytes && !g_app.stopRequested.load()) {
                        if (g_app.pauseRequested.load()) {
                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                            PostMessageW(g_app.hMain, WM_NULL, 0, 0);
                            auto pauseStart = std::chrono::steady_clock::now();
                            while (g_app.pauseRequested.load()) {
                                if (g_app.stopRequested.load()) break;
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                            auto pauseEnd = std::chrono::steady_clock::now();
                            pausedSeconds += std::chrono::duration<double>(pauseEnd - pauseStart).count();
                            lastUpdate = pauseEnd;
                            if (g_app.stopRequested.load()) break;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(50));

                        // Simulate random download speed (approx 5 MB/s to 150 MB/s)
                        uint64_t chunk = (static_cast<uint64_t>(5000) + (rand() % 145000)) * 1024ULL / 20ULL;
                        downloadedBytes += chunk;
                        if (downloadedBytes > totalBytes) downloadedBytes = totalBytes;

                        auto now = std::chrono::steady_clock::now();
                        auto elapsedSec = std::chrono::duration<double>(now - startTime).count() - pausedSeconds;
                        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();

                        if (dt >= 100 || downloadedBytes == totalBytes) {
                            double speedMbps = 0.0;
                            if (elapsedSec > 0.0) {
                                speedMbps = (downloadedBytes / 1024.0 / 1024.0) / elapsedSec;
                            }

                            double dlMb = downloadedBytes / 1024.0 / 1024.0;
                            double totMb = totalBytes / 1024.0 / 1024.0;

                            wchar_t buf[256];
                            swprintf_s(buf, L"Ã Â¢Ã ÂµÃ ÂºÃ‘Â Ã‘â€š Ã Â·Ã Â°Ã Â³Ã‘â‚¬Ã‘Æ’Ã Â·Ã ÂºÃ Â¸: %s / %s   |   Ã Â¡Ã ÂºÃ Â¾Ã‘â‚¬Ã Â¾Ã‘Â Ã‘â€šÃ‘Å’: %.1f Ã Å“Ã â€˜/Ã‘Â ",
                                FormatSizeStr(dlMb).c_str(), FormatSizeStr(totMb).c_str(), speedMbps);

                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = buf; }
                            g_app.downloadProgress = (float)((double)downloadedBytes / (double)totalBytes);

                            InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);
                            lastUpdate = now;
                        }
                    }

                    g_app.downloadProgress = 1.0f;
                    g_app.downloadProgressSmooth = 1.0f;
                    if (g_app.stopRequested.load()) {
                        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                    }
                    else {
                        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadStatsText = L"Подождите..."; }
                    }
                    InvalidateRect(g_app.hDownloadPanel, nullptr, FALSE);

                    g_app.isDownloading = false;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    HideDownloadPanelFade();
                    g_app.stopRequested = false;
                    });
            }
            break;
#endif
        case ID_NAV_BACK: {
            if (g_app.inSubMenu) {
                ShowPage(Page::Overview);
                g_app.inSubMenu = false;
                g_app.menuIsAnimating = true;
                g_app.navIndicatorY = g_app.navIndicatorTargetY;
                SetTimer(g_app.hSidebar, 2, 16, nullptr);
            }
            break;
        }
        case ID_NAV_MODE_LIGHTING:
        case ID_NAV_MODE_FULL: {
            {
                std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
                if (id == ID_NAV_MODE_LIGHTING) {
                    if (g_app.rtxSelectedIndex != 0) {
                        g_app.rtxSelectedIndex = 0;

                    }
                }
                else {
                    g_app.rtxSelectedIndex = 1;
                }
            }
            UpdateRtxUI();
            g_app.navIndicatorTargetY = (id == ID_NAV_MODE_LIGHTING) ? 156.0f : 202.0f;
            SetTimer(g_app.hSidebar, 6, 16, nullptr);
            InvalidateRect(g_app.hNavModeLighting, nullptr, FALSE);
            InvalidateRect(g_app.hNavModeFull, nullptr, FALSE);
            break;
        }
        case ID_BTN_OPEN_DEST: {
            std::wstring dst = g_app.installRootPath;
            if (dst.empty()) break;
            std::error_code ec;
            if (!fs::is_directory(dst, ec)) { AppendLog(L"Ã Å¡Ã‘â‚¬Ã Â¸Ã‘â€šÃ Â¸Ã‘â€¡Ã ÂµÃ‘Â Ã ÂºÃ Â°Ã‘Â  Ã Â¾Ã‘Ë†Ã Â¸Ã Â±Ã ÂºÃ Â°: не Ã‘Â Ã‘Æ’Ã‘â€°Ã ÂµÃ‘Â Ã‘â€šÃ Â²Ã‘Æ’Ã ÂµÃ‘â€š: " + dst); break; }
            ShellExecuteW(hwnd, L"open", dst.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            break;
        }
        case ID_BTN_LAUNCH_MODE:
            g_app.launchMode = (g_app.launchMode == 2) ? 1 : 2;
            SetWindowTextW(g_app.hBtnLaunchMode, g_app.launchMode == 2 ? L"Режим: Ð¡Ð¾Ð²Ð¼ÐµÑ Ñ‚Ð¸Ð¼Ð¾Ñ Ñ‚ÑŒ" : L"Режим: Обычный");
            SaveSettings();
            break;
        case ID_BTN_REPAIR:
            g_app.optRepair = !g_app.optRepair;
            Theme::g_buttonStates[g_app.hBtnRepair].accent = g_app.optRepair;
            InvalidateRect(g_app.hBtnRepair, nullptr, FALSE);
            break;

        case ID_BTN_SYNC: DoSync(); break;
        case ID_BTN_CHANGE_DISK:
            g_app.oldInstallRootPath = g_app.installRootPath;
            ShowDiskSelectionModal(OnDiskChanged);
            break;
        case ID_BTN_LAUNCH: DoLaunchGame(); break;
        case ID_BTN_APPLY_RTX: DoApplyRtxVersion(); break;
        case ID_BTN_GITHUB_RTX: DoOpenRtxGithub(); break;
        case ID_BTN_SYS_MIN:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        case ID_BTN_SYS_CLOSE:
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_BTN_REFRESH_RTX: {
            if (lParam == 1) { // 1 = background thread UI update
                UpdateRtxUI();
            }
            else {
                LoadRtxReleases();
            }
            break;
        }
        case ID_BTN_PAUSE: {
            g_app.pauseRequested = !g_app.pauseRequested;
            if (g_app.pauseRequested) {
                SetWindowTextW(g_app.hBtnPauseDownload, L"Продолжить");
            }
            else {
                SetWindowTextW(g_app.hBtnPauseDownload, L"Пауза");
            }
            break;
        }
        case ID_BTN_STOP: DoStop(); break;
        default:
            if (id >= ID_LIST_ITEM_FIRST && id <= ID_LIST_ITEM_LAST) {
                int idx = id - ID_LIST_ITEM_FIRST;
                {
                    std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
                    if (idx < 2) {
                        g_app.rtxSelectedIndex = idx;
                    }
                }
                UpdateRtxUI();
            }
            break;
        }
        return 0;
    }
    case WM_DESTROY:
        SaveSettings();
        DeleteObject(g_app.brBgMain);
        DeleteObject(g_app.brBgPanel);
        DeleteObject(g_app.brBgInput);
        DeleteObject(g_app.brBgConsole);
        DeleteObject(g_app.brSidebar);
        DeleteObject(g_fontUI);
        DeleteObject(g_fontHeading);
        DeleteObject(g_fontTitle);
        DeleteObject(g_fontSmall);
        DeleteObject(g_fontMono);
        PostQuitMessage(0);
        return 0;
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProcW(hwnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = {};
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hwnd, &pt);
            RECT r;
            GetClientRect(hwnd, &r);
            int winW = r.right - r.left;
            if (pt.y < 32 && pt.x > 70 && pt.x < winW - 96) {
                return HTCAPTION;
            }
        }
        return hit;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


// --- ImGui & D3D11 Globals ---
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;


bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProcImGui(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            UINT w = (UINT)LOWORD(lParam);
            UINT h = (UINT)HIWORD(lParam);
            g_ResizeWidth = w;
            g_ResizeHeight = h;
            if (w > 0 && h > 0) {
                HRGN hRgn = CreateRoundRectRgn(0, 0, (int)w + 1, (int)h + 1, 24, 24);
                SetWindowRgn(hWnd, hRgn, TRUE);
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProcW(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = {};
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            RECT r;
            GetClientRect(hWnd, &r);
            int winW = r.right - r.left;
            if (pt.y < 32 && pt.x > 70 && pt.x < winW - 96) {
                return HTCAPTION;
            }
        }
        return hit;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

// Global ImGui fonts
ImFont* g_imFontTitle = nullptr;
ImFont* g_imFontHeading = nullptr;
ImFont* g_imFontRegular = nullptr;
ImFont* g_imFontSmall = nullptr;

static void DoOpenDest() {
    std::wstring path = g_app.installRootPath;
    if (!path.empty()) {
        ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
    }
}

static void ApplyRtxRemixStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 0.0f;
    style.FrameRounding = 20.0f; // Pill buttons
    style.GrabRounding = 20.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;

    style.FramePadding = ImVec2(16, 8);
    style.ItemSpacing = ImVec2(12, 12);

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.47f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.46f, 0.72f, 0.00f, 0.80f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.52f, 0.81f, 0.00f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.62f, 0.00f, 1.00f);
}

void PushAccentButton() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.90f, 0.46f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 1.00f, 0.52f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.78f, 0.40f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.08f, 0.06f, 1.00f));
}
void PopAccentButton() {
    ImGui::PopStyleColor(4);
}




enum class AnimState { Waiting, Resizing, FadingIn, Done };
static AnimState g_animState = AnimState::Waiting;
static float g_animTimer = 0.0f;
float g_uiAlpha = 1.0f;

float g_pageTransitionAlpha = 1.0f;
static Page g_targetPage = Page::Overview;
bool g_isPageTransitioning = false;
bool g_isSmoothScrolling = false;
bool g_isSidebarAnimating = false;
struct WindowAnim {
    bool active = false;
    float startW = 0.0f, startH = 0.0f, startX = 0.0f, startY = 0.0f;
    float endW = 0.0f, endH = 0.0f, endX = 0.0f, endY = 0.0f;
    float timer = 0.0f;
    float duration = 0.4f;
} g_winAnim;

float g_uiScale = 1.0f;

static void CalculateUiScale(float& outW, float& outH) {
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    float scale = (float)screenH / 1080.0f;
    if (scale < 0.85f) scale = 0.85f;
    if (scale > 2.0f) scale = 2.0f;

    g_uiScale = scale;
    outW = 860.0f * scale;
    outH = 500.0f * scale;
}

bool IsAnimationActive() {
    if (g_isSmoothScrolling) return true;
    if (g_wizardIsSliding) return true;
    if (g_isSidebarAnimating) return true;
    if (g_winAnim.active) return true;
    if (g_isPageTransitioning) return true;
    if (g_pageTransitionAlpha > 0.0f && g_pageTransitionAlpha < 1.0f) return true;
    if (g_diskModalState != DiskModalState::Closed) return true;
    if (g_launchModalState != LaunchModeModalState::Closed) return true;
    if (g_app.isDownloading.load()) return true;
    if (fabs(g_app.downloadProgressSmooth - g_app.downloadProgress) > 0.001f) return true;
    if (g_app.currentPage == Page::UpdateCheck) return true;
    if (g_app.currentPage == Page::InstallerWizard) return true;
    return false;
}

ImVec4 GetAdaptiveProgressColor(float progress) {
    float t = progress;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float r, g, b;
    if (t < 0.5f) {
        float localT = t * 2.0f; // 0.0 .. 1.0
        r = 0.92f;
        g = 0.25f + 0.55f * localT; // 0.25 (Красный) -> 0.80 (Желтый)
        b = 0.20f;
    } else {
        float localT = (t - 0.5f) * 2.0f; // 0.0 .. 1.0
        r = 0.92f - 0.62f * localT; // 0.92 (Желтый) -> 0.30 (Зеленый)
        g = 0.80f + 0.05f * localT; // 0.80 -> 0.85
        b = 0.20f + 0.19f * localT; // 0.20 -> 0.39
    }
    return ImVec4(r, g, b, 1.0f);
}

static inline float S(float v) { return v * g_uiScale; }
static inline ImVec2 S(float x, float y) { return ImVec2(x * g_uiScale, y * g_uiScale); }

void SwitchPage(Page page) {
    if (g_app.currentPage == page) return;
    g_targetPage = page;
    g_isPageTransitioning = true;
    if (g_app.currentPage == Page::UpdateCheck && (page == Page::Overview || page == Page::InstallerWizard)) {
        RECT r;
        GetWindowRect(g_app.hMain, &r);
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        g_winAnim.active = true;
        g_winAnim.timer = 0.0f;
        g_winAnim.duration = 0.4f;

        g_winAnim.startX = (float)r.left;
        g_winAnim.startY = (float)r.top;
        g_winAnim.startW = (float)(r.right - r.left);
        g_winAnim.startH = (float)(r.bottom - r.top);

        float targetW, targetH;
        CalculateUiScale(targetW, targetH);

        g_winAnim.endW = targetW;
        g_winAnim.endH = targetH;
        g_winAnim.endX = (screenW - targetW) / 2.0f;
        g_winAnim.endY = (screenH - targetH) / 2.0f;
    }

}

void RenderSingleWizardStep(WizardStep currentStepToDraw, float childW) {
    bool isRunning = g_app.operationRunning.load();
    if (currentStepToDraw == WizardStep::Welcome) {
        ImGui::PushFont(g_imFontHeading);
        ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), u8"Добро пожаловать в Metrostroi RTX Remixed!");
        ImGui::PopFont();
        ImGui::Spacing();
        ImGui::PushTextWrapPos(childW - 30.0f);
        ImGui::TextWrapped(u8"Данная модификация представляет собой глобальное графическое переосмысление культового симулятора метро Garry's Mod Metrostroi на современном движке трассировки путей NVIDIA RTX Remix.");
        ImGui::Spacing();
        ImGui::TextDisabled(u8"КЛЮЧЕВЫЕ ОСОБЕННОСТИ:");
        ImGui::BulletText(u8"Реалистичный просчёт физического света (Path Tracing) в тоннелях и на станциях");
        ImGui::BulletText(u8"Переработанные материалы вагонов 81-717, Еж3 и станций с честными отражениями PBR");
        ImGui::BulletText(u8"Адаптированный высокопроизводительный гибридный рендерер DXVK RTX");
        ImGui::PopTextWrapPos();

        ImGui::Spacing(); ImGui::Spacing();
        PushAccentButton();
        if (ImGui::Button(u8"Далее  →", ImVec2(180, 42))) {
            GoToWizardStep(WizardStep::LaunchMode);
        }
        PopAccentButton();
    }
    else if (currentStepToDraw == WizardStep::LaunchMode) {
        ImGui::PushFont(g_imFontHeading);
        ImGui::Text(u8"Выберите режим запуска Garry's Mod:");
        ImGui::PopFont();
        ImGui::Spacing();

        ImVec2 cardSize(childW - 30.0f, 65.0f);
        ImVec2 basePos = ImGui::GetCursorPos();
        ImVec2 baseScreenPos = ImGui::GetCursorScreenPos();
        
        static float s_lmHlY = 0.0f;
        static float s_lmHlAlpha = 0.0f;
        
        float targetY = 0.0f;
        float targetAlpha = (g_app.launchMode > 0) ? 1.0f : 0.0f;
        if (g_app.launchMode == 1) targetY = 0.0f;
        else if (g_app.launchMode == 2) targetY = cardSize.y + 12.0f;
        
        if (g_app.launchMode > 0 && s_lmHlAlpha < 0.01f) {
            s_lmHlY = targetY; // snap
        }
        
        s_lmHlY += (targetY - s_lmHlY) * ImGui::GetIO().DeltaTime * 15.0f;
        s_lmHlAlpha += (targetAlpha - s_lmHlAlpha) * ImGui::GetIO().DeltaTime * 15.0f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        // Card 1 Base
        ImVec2 pMin1 = baseScreenPos;
        ImVec2 pMax1 = ImVec2(pMin1.x + cardSize.x, pMin1.y + cardSize.y);
        bool hov1 = ImGui::IsMouseHoveringRect(pMin1, pMax1);
        ImU32 bg1 = hov1 ? IM_COL32(36, 44, 56, 160) : IM_COL32(24, 28, 36, 120);
        dl->AddRectFilled(pMin1, pMax1, bg1, 10.0f);
        dl->AddRect(pMin1, pMax1, IM_COL32(50, 60, 75, 90), 10.0f, 0, 1.0f);

        // Card 2 Base
        ImVec2 pMin2 = ImVec2(baseScreenPos.x, baseScreenPos.y + cardSize.y + 12.0f);
        ImVec2 pMax2 = ImVec2(pMin2.x + cardSize.x, pMin2.y + cardSize.y);
        bool hov2 = ImGui::IsMouseHoveringRect(pMin2, pMax2);
        ImU32 bg2 = hov2 ? IM_COL32(36, 44, 56, 160) : IM_COL32(24, 28, 36, 120);
        dl->AddRectFilled(pMin2, pMax2, bg2, 10.0f);
        dl->AddRect(pMin2, pMax2, IM_COL32(50, 60, 75, 90), 10.0f, 0, 1.0f);

        // Draw Sliding Highlight
        if (s_lmHlAlpha > 0.01f) {
            ImVec2 hMin = ImVec2(baseScreenPos.x, baseScreenPos.y + s_lmHlY);
            ImVec2 hMax = ImVec2(hMin.x + cardSize.x, hMin.y + cardSize.y);
            ImU32 hBg = IM_COL32(0, 60, 32, (int)(180 * s_lmHlAlpha));
            ImU32 hBrd = IM_COL32(0, 230, 118, (int)(220 * s_lmHlAlpha));
            dl->AddRectFilled(hMin, hMax, hBg, 10.0f);
            dl->AddRect(hMin, hMax, hBrd, 10.0f, 0, 1.5f);
        }

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));

        // Card 1 Content
        ImGui::SetCursorPos(basePos);
        ImGui::PushID(201);
        if (ImGui::Selectable("##ModeCard1", (g_app.launchMode == 1), ImGuiSelectableFlags_AllowOverlap, cardSize)) {
            g_app.launchMode = 1;
            SaveSettings();
        }
        ImGui::SetCursorPos(ImVec2(basePos.x + 14.0f, basePos.y + 8.0f));
        ImGui::PushFont(g_imFontRegular);
        ImGui::Text(u8"● Обычный режим (По умолчанию)");
        ImGui::PopFont();
        ImGui::SetCursorPos(ImVec2(basePos.x + 14.0f, basePos.y + 32.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.74f, 0.82f, 1.0f));
        ImGui::TextWrapped(u8"Стандартный запуск Garry's Mod x64 с гибридной подсистемой RTX Remix.");
        ImGui::PopStyleColor();
        ImGui::PopID();

        // Card 2 Content
        ImVec2 pos2 = ImVec2(basePos.x, basePos.y + cardSize.y + 12.0f);
        ImGui::SetCursorPos(pos2);
        ImGui::PushID(202);
        if (ImGui::Selectable("##ModeCard2", (g_app.launchMode == 2), ImGuiSelectableFlags_AllowOverlap, cardSize)) {
            g_app.launchMode = 2;
            SaveSettings();
        }
        ImGui::SetCursorPos(ImVec2(pos2.x + 14.0f, pos2.y + 8.0f));
        ImGui::PushFont(g_imFontRegular);
        ImGui::Text(u8"● Режим повышенной совместимости (-high +mat_dxlevel 95)");
        ImGui::PopFont();
        ImGui::SetCursorPos(ImVec2(pos2.x + 14.0f, pos2.y + 32.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.74f, 0.82f, 1.0f));
        ImGui::TextWrapped(u8"Приоритет Windows (-high) и принудительный DirectX 9.5 (+mat_dxlevel 95).");
        ImGui::PopStyleColor();
        ImGui::PopID();

        ImGui::PopStyleColor(3);

        ImGui::SetCursorPos(ImVec2(pos2.x, pos2.y + cardSize.y + 16.0f));

        ImGui::Spacing(); ImGui::Spacing();
        bool canProceed = (g_app.launchMode > 0);
        PushAccentButton();
        if (!canProceed) ImGui::BeginDisabled();
        if (ImGui::Button(u8"Далее  →", ImVec2(180, 42))) {
            GoToWizardStep(WizardStep::DriveSelect);
        }
        if (!canProceed) ImGui::EndDisabled();
        PopAccentButton();
    }
    else if (currentStepToDraw == WizardStep::DriveSelect) {
        ImGui::TextDisabled(u8"ВЫБЕРИТЕ НАКОПИТЕЛЬ ДЛЯ РАЗВЕРТЫВАНИЯ ИГРЫ:");
        ImGui::Spacing();

        if (g_app.availableDisks.empty()) {
            g_app.availableDisks.clear();
            DWORD drives = GetLogicalDrives();
            for (char c = 'A'; c <= 'Z'; ++c) {
                if (drives & (1 << (c - 'A'))) {
                    std::wstring root = std::wstring(1, c) + L":\\";
                    UINT type = GetDriveTypeW(root.c_str());
                    if (type == DRIVE_FIXED) {
                        ULARGE_INTEGER freeBytes, totalBytes;
                        if (GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, nullptr)) {
                            DiskInfo info;
                            info.path = root;
                            info.name = (c == 'C') ? L"Локальный диск" : (L"Диск " + std::wstring(1, c));
                            info.totalSpace = totalBytes.QuadPart;
                            info.freeSpace = freeBytes.QuadPart;
                            info.isSSD = (c == 'C');
                            info.isHDD = !info.isSSD;
                            g_app.availableDisks.push_back(info);
                        }
                    }
                }
            }
        }

        ImVec2 basePos = ImGui::GetCursorPos();
        ImVec2 baseScreenPos = ImGui::GetCursorScreenPos();
        ImVec2 diskCardSize(childW - 30.0f, 42.0f);
        
        int selectedIndex = -1;
        float targetY = 0.0f;
        
        for (size_t i = 0; i < g_app.availableDisks.size(); ++i) {
            const auto& disk = g_app.availableDisks[i];
            bool isSelected = (g_app.installRootPath == (disk.path + L"Metrostroi RTX"));
            if (isSelected) {
                selectedIndex = (int)i;
                targetY = ImGui::GetCursorPos().y - basePos.y;
            }
            
            ImVec2 pMinD = ImGui::GetCursorScreenPos();
            ImVec2 pMaxD = ImVec2(pMinD.x + diskCardSize.x, pMinD.y + diskCardSize.y);
            bool hovD = ImGui::IsMouseHoveringRect(pMinD, pMaxD);
            ImU32 bgD = hovD ? IM_COL32(36, 44, 56, 160) : IM_COL32(24, 28, 36, 120);
            
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(pMinD, pMaxD, bgD, 10.0f);
            dl->AddRect(pMinD, pMaxD, IM_COL32(50, 60, 75, 90), 10.0f, 0, 1.0f);
            
            ImGui::Dummy(diskCardSize);
            ImGui::Spacing();
        }
        
        static float s_dsHlY = 0.0f;
        static float s_dsHlAlpha = 0.0f;
        float targetAlpha = (selectedIndex >= 0) ? 1.0f : 0.0f;
        
        if (selectedIndex >= 0 && s_dsHlAlpha < 0.01f) {
            s_dsHlY = targetY; // snap
        }
        
        s_dsHlY += (targetY - s_dsHlY) * ImGui::GetIO().DeltaTime * 15.0f;
        s_dsHlAlpha += (targetAlpha - s_dsHlAlpha) * ImGui::GetIO().DeltaTime * 15.0f;
        
        if (s_dsHlAlpha > 0.01f) {
            ImVec2 hMin = ImVec2(baseScreenPos.x, baseScreenPos.y + s_dsHlY);
            ImVec2 hMax = ImVec2(hMin.x + diskCardSize.x, hMin.y + diskCardSize.y);
            ImU32 hBg = IM_COL32(0, 60, 32, (int)(180 * s_dsHlAlpha));
            ImU32 hBrd = IM_COL32(0, 230, 118, (int)(220 * s_dsHlAlpha));
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(hMin, hMax, hBg, 10.0f);
            dl->AddRect(hMin, hMax, hBrd, 10.0f, 0, 1.5f);
        }
        
        ImGui::SetCursorPos(basePos);
        
        for (size_t i = 0; i < g_app.availableDisks.size(); ++i) {
            const auto& disk = g_app.availableDisks[i];
            double freeGb = (double)disk.freeSpace / (1024.0 * 1024.0 * 1024.0);
            double totalGb = (double)disk.totalSpace / (1024.0 * 1024.0 * 1024.0);
            std::string utf8_path = WStringToUTF8(disk.path);
            
            char label[256];
            snprintf(label, sizeof(label), "  Диск %s  |  Св. %.1f ГБ из %.1f ГБ %s##inst_%d",
                utf8_path.c_str(), freeGb, totalGb, disk.isSSD ? "(SSD - Рек.)" : "(HDD)", (int)i);
                
            bool isSelected = (g_app.installRootPath == (disk.path + L"Metrostroi RTX"));
            
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
            
            if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowOverlap, diskCardSize)) {
                g_app.installRootPath = disk.path + L"Metrostroi RTX";
                SaveSettings();
            }
            ImGui::PopStyleColor(3);
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        std::string currentPathUtf8 = WStringToUTF8(g_app.installRootPath);
        if (currentPathUtf8.empty()) currentPathUtf8 = u8"Папка не выбрана (нажмите на диск выше)";
        ImGui::TextDisabled(u8"ПУТЬ УСТАНОВКИ:");
        ImGui::TextColored(ImVec4(0.0f, 0.82f, 1.0f, 1.0f), "%s", currentPathUtf8.c_str());

        ImGui::Spacing(); ImGui::Spacing();
        bool canProceed = !g_app.installRootPath.empty();
        PushAccentButton();
        if (!canProceed || isRunning) ImGui::BeginDisabled();
        if (ImGui::Button(u8"НАЧАТЬ УСТАНОВКУ", ImVec2(240, 42))) {
            g_autoStartGameAfterInstall = false;
            GoToWizardStep(WizardStep::Progress);
            DoLaunchGame();
        }
        if (!canProceed || isRunning) ImGui::EndDisabled();
        PopAccentButton();
    }
    else if (currentStepToDraw == WizardStep::Progress) {
        ImGui::PushFont(g_imFontHeading);
        ImGui::Text(u8"Идёт развертывание и патчинг компонента RTX...");
        ImGui::PopFont();
        ImGui::Spacing();
        ImGui::TextDisabled(u8"Пожалуйста, подождите. По завершении игра НЕ будет запущена автоматически.");
        ImGui::Spacing(); ImGui::Spacing();

        std::string statsUtf8 = WStringToUTF8(g_app.downloadStatsText);
        std::string titleUtf8 = WStringToUTF8(g_app.downloadTitleText);
        ImGui::TextColored(ImVec4(0.0f, 0.82f, 1.0f, 1.0f), "%s", titleUtf8.c_str());
        ImGui::Text("%s", statsUtf8.c_str());
        ImGui::Spacing();

        ImVec4 barColor = GetAdaptiveProgressColor(g_app.downloadProgressSmooth);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        ImGui::ProgressBar(g_app.downloadProgressSmooth, ImVec2(childW - 30.0f, 24), "");
        ImGui::PopStyleColor();
    }
    else if (currentStepToDraw == WizardStep::Complete) {
        ImGui::PushFont(g_imFontHeading);
        ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), u8"Установка успешно завершена!");
        ImGui::PopFont();
        ImGui::Spacing();
        ImGui::TextWrapped(u8"Модификация Metrostroi RTX Remixed полностью установлена и готова к работе!");
        ImGui::Spacing(); ImGui::Spacing();

        PushAccentButton();
        if (ImGui::Button(u8"ИГРАТЬ СЕЙЧАС", ImVec2(210, 48))) {
            g_autoStartGameAfterInstall = true;
            g_sidebarAnimState = SidebarAnimState::WizardOut_MenuIn;
            g_sidebarAnimTimer = 0.0f;
            SwitchPage(Page::Overview);
            DoLaunchGame();
        }
        PopAccentButton();

        ImGui::SameLine(240);
        if (ImGui::Button(u8"ПОЗЖЕ", ImVec2(170, 48))) {
            g_sidebarAnimState = SidebarAnimState::WizardOut_MenuIn;
            g_sidebarAnimTimer = 0.0f;
            SwitchPage(Page::Overview);
        }
    }
}

static void RenderImGuiUI() {
    g_isSmoothScrolling = false;
    float dummyW, dummyH;
    CalculateUiScale(dummyW, dummyH);
    ImGui::GetIO().FontGlobalScale = g_uiScale;

    bool isRunning = g_app.operationRunning.load();

    // Smoothly interpolate progress bar to prevent abrupt jumps
    float targetProgress = g_app.downloadProgress;
    if (g_app.downloadProgressSmooth != targetProgress) {
        float diff = targetProgress - g_app.downloadProgressSmooth;
        if (fabs(diff) < 0.0001f) {
            g_app.downloadProgressSmooth = targetProgress;
        }
        else {
            float dt = ImGui::GetIO().DeltaTime;
            float factor = dt * 5.0f; // Lerp factor
            if (factor > 1.0f) factor = 1.0f;
            g_app.downloadProgressSmooth += diff * factor;
        }
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin("RTX Launcher", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);



    if (g_isPageTransitioning) {
        g_pageTransitionAlpha -= ImGui::GetIO().DeltaTime / 0.15f;
        if (g_pageTransitionAlpha <= 0.0f) {
            g_pageTransitionAlpha = 0.0f;
            g_app.currentPage = g_targetPage;
            g_isPageTransitioning = false;
        }
    }
    else if (g_pageTransitionAlpha < 1.0f) {
        g_pageTransitionAlpha += ImGui::GetIO().DeltaTime / 0.15f;
        if (g_pageTransitionAlpha > 1.0f) g_pageTransitionAlpha = 1.0f;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_uiAlpha * g_pageTransitionAlpha);



    // Hide UI during window expansion animation
    if (g_winAnim.active) {
        ImGui::PopStyleVar(); // pop alpha
        ImGui::End();
        return;
    }

    // Show UpdateCheck UI before animation (single rounded window)
    if (g_app.currentPage == Page::UpdateCheck) {
        static float timer = 0.0f;
        static bool hasChecked = false;
        static bool updateStarted = false;

        timer += ImGui::GetIO().DeltaTime;
        if (timer >= 0.1f && !hasChecked) {
            hasChecked = true;
            CheckLauncherUpdatesAsync();
        }
        
        bool checkFinished = !g_isCheckingLauncherUpdate.load() && timer >= 0.5f;

        if (!updateStarted && checkFinished && g_launcherUpdateInfo.hasUpdate && !g_launcherUpdateInfo.downloadUrl.empty()) {
            updateStarted = true;
            g_app.isDownloading = true;
            g_app.downloadProgress = 0.0f;
            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Обновление лаунчера..."; }
            
            RunInBackground([]() {
                LauncherUpdater::DownloadAndApplyUpdate(g_launcherUpdateInfo.downloadUrl,
                    g_launcherUpdateInfo.releaseNotes,
                    [](const std::wstring& msg) { AppendLog(msg); },
                    [](float p) {
                        g_app.downloadProgress = p;
                        { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Обновление лаунчера..."; }
                    },
                    g_app.githubToken);
            });
        }

        // Fade out after 2.0 seconds over 0.5 seconds (only if no update)
        float alpha = 1.0f;
        if (!updateStarted && timer >= 2.0f) {
            alpha = std::max(0.0f, 1.0f - (timer - 2.0f) / 0.5f);
        }

        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 center = ImVec2(winSize.x * 0.5f, (winSize.y + 32.0f) * 0.5f);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_uiAlpha * g_pageTransitionAlpha * alpha);
        ImGui::PushFont(g_imFontHeading);
        
        std::string text;
        if (updateStarted) {
            int pct = (int)(g_app.downloadProgress * 100.0f);
            text = u8"Загрузка обновления... " + std::to_string(pct) + u8"%";
        } else {
            text = u8"Проверка на наличие обновлений...";
        }

        float textWidth = ImGui::CalcTextSize(text.c_str()).x;
        float textHeight = ImGui::GetFontSize();
        ImGui::SetCursorPos(ImVec2(center.x - textWidth * 0.5f, center.y - textHeight * 0.5f));
        ImGui::Text("%s", text.c_str());
        
        if (updateStarted) {
            float barWidth = 300.0f;
            ImGui::SetCursorPos(ImVec2(center.x - barWidth * 0.5f, center.y + textHeight * 0.5f + 15.0f));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, GetAdaptiveProgressColor(g_app.downloadProgress));
            ImGui::ProgressBar(g_app.downloadProgress, ImVec2(barWidth, 6.0f), "");
            ImGui::PopStyleColor();
        }

        ImGui::PopFont();
        ImGui::PopStyleVar(); // fade alpha

        ImGui::PopStyleVar(); // main alpha
        ImGui::End();

        if (!updateStarted && ((timer >= 1.5f && checkFinished) || timer >= 4.0f)) {
            std::error_code ec;
            bool hasInstallation = !g_app.installRootPath.empty() && 
                                   (fs::exists(g_app.installRootPath + L"\\gmod.exe", ec) || fs::exists(g_app.installRootPath + L"\\hl2.exe", ec));
            
            if (hasInstallation) {
                SwitchPage(Page::Overview);
            } else {
                SwitchPage(Page::InstallerWizard);
            }
            timer = 0.0f; // reset for future checks
        }
        return;
    }

    // --- Sidebar & Layout ---
    if (g_app.currentPage != Page::UpdateCheck) {
        ImGui::BeginGroup(); // Sidebar Group
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.08f, 0.10f, 1.0f));
        ImGui::BeginChild("Sidebar", ImVec2(S(190.0f), 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        // MacOS Dots
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        // Close (Red)
        ImGui::SetCursorPos(S(10, 10));
        if (ImGui::InvisibleButton("CloseBtn", S(14, 14))) PostQuitMessage(0);
        bool closeHovered = ImGui::IsItemHovered();
        ImVec2 rMinC = ImGui::GetItemRectMin();
        ImVec2 rMaxC = ImGui::GetItemRectMax();
        ImVec2 centerC(rMinC.x + (rMaxC.x - rMinC.x) * 0.5f, rMinC.y + (rMaxC.y - rMinC.y) * 0.5f);
        dl->AddCircleFilled(centerC, S(6.0f), IM_COL32(255, 95, 86, 255));
        if (closeHovered) {
            float s = S(2.5f);
            dl->AddLine(ImVec2(centerC.x - s, centerC.y - s), ImVec2(centerC.x + s, centerC.y + s), IM_COL32(76, 0, 0, 255), S(1.5f));
            dl->AddLine(ImVec2(centerC.x + s, centerC.y - s), ImVec2(centerC.x - s, centerC.y + s), IM_COL32(76, 0, 0, 255), S(1.5f));
        }
        
        // Minimize (Yellow)
        ImGui::SetCursorPos(S(30, 10));
        if (ImGui::InvisibleButton("MinBtn", S(14, 14))) ShowWindow(g_app.hMain, SW_MINIMIZE);
        bool minHovered = ImGui::IsItemHovered();
        ImVec2 rMinM = ImGui::GetItemRectMin();
        ImVec2 rMaxM = ImGui::GetItemRectMax();
        ImVec2 centerM(rMinM.x + (rMaxM.x - rMinM.x) * 0.5f, rMinM.y + (rMaxM.y - rMinM.y) * 0.5f);
        dl->AddCircleFilled(centerM, S(6.0f), IM_COL32(255, 189, 46, 255));
        if (minHovered) {
            float s = S(3.0f);
            dl->AddLine(ImVec2(centerM.x - s, centerM.y), ImVec2(centerM.x + s, centerM.y), IM_COL32(100, 60, 0, 255), S(1.5f));
        }


        
        // Handle Sidebar Animation Timer
        if (g_sidebarAnimState == SidebarAnimState::WizardOut_MenuIn || g_sidebarAnimState == SidebarAnimState::SubMenuTransition) {
            g_sidebarAnimTimer += ImGui::GetIO().DeltaTime * 1.5f;
            if (g_sidebarAnimTimer >= 1.0f) {
                g_sidebarAnimTimer = 1.0f;
                g_sidebarAnimState = SidebarAnimState::None;
            }
        }

        // Highlight sliding animation
        static float s_navHighlightY = 60.0f;
        static float s_navHighlightH = 45.0f;
        float targetY = 60.0f;
        float targetH = 45.0f;
        
        if (g_app.currentPage == Page::InstallerWizard) {
            targetY = 60.0f + static_cast<int>(g_wizardStep) * 55.0f;
        } else if (g_app.currentPage == Page::Authors) {
            targetY = (ImGui::GetWindowHeight() / g_uiScale) - 34.0f;
            targetH = 22.0f;
        } else {
            if (g_sidebarMenu == SidebarMenu::Main) {
                if (g_app.currentPage == Page::Overview) targetY = 60.0f;
                else if (g_app.currentPage == Page::Settings) targetY = 60.0f + 55.0f;
                else if (g_app.currentPage == Page::RtxMods) targetY = 60.0f + 110.0f;
            } else if (g_sidebarMenu == SidebarMenu::RtxGames) {
                if (g_app.currentPage == Page::RtxMods) targetY = 60.0f + 55.0f;
                else targetY = 60.0f; // Highlight back button if they somehow are here without RtxMods active
            }
        }
        
        s_navHighlightY += (targetY - s_navHighlightY) * ImGui::GetIO().DeltaTime * 15.0f;
        s_navHighlightH += (targetH - s_navHighlightH) * ImGui::GetIO().DeltaTime * 15.0f;
        g_isSidebarAnimating = fabs(targetY - s_navHighlightY) > 0.01f || fabs(targetH - s_navHighlightH) > 0.01f || g_sidebarAnimState != SidebarAnimState::None;
        
        ImVec2 windowPos = ImGui::GetWindowPos();
        float currentY_screen = windowPos.y + S(s_navHighlightY);
        
        // Compute staggered offset for highlight
        float highlightOffsetX = 0.0f;
        if (g_sidebarAnimState == SidebarAnimState::WizardOut_MenuIn) {
            // Highlight box rapidly slides up to Overview during transition
            float ease = 1.0f - powf(1.0f - g_sidebarAnimTimer, 3.0f);
            s_navHighlightY = 60.0f * ease + s_navHighlightY * (1.0f - ease);
        }

        dl->AddRectFilled(ImVec2(windowPos.x + S(16.0f) + highlightOffsetX, currentY_screen), 
                          ImVec2(windowPos.x + S(16.0f) + S(158.0f) + highlightOffsetX, currentY_screen + S(s_navHighlightH)),
                          IM_COL32(13, 51, 38, 255), S(8.0f));

        ImGui::SetCursorPos(S(16, 60));
        ImGui::PushFont(g_imFontRegular);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, S(10.0f))); // Exact spacing for precise calculation
        auto NavButton = [](const char* label, bool active, float xOffset, bool interactive = true) {
            ImGui::SetCursorPosX(S(16.0f) + xOffset);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            
            if (interactive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.03f)); 
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1,1,1,0.06f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0)); 
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));
            }
            
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.90f, 0.46f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            }
            bool clicked = ImGui::Button(label, ImVec2(S(158), S(45)));
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(1);
            return clicked && interactive;
        };

        auto GetStaggeredOffset = [](int index, bool isWizard, bool isSubMenuTransition, bool isEntering) {
            if (g_sidebarAnimState != SidebarAnimState::WizardOut_MenuIn && g_sidebarAnimState != SidebarAnimState::SubMenuTransition) return 0.0f;
            
            float delay = index * 0.1f;
            float t = g_sidebarAnimTimer - delay;
            if (t < 0.0f) t = 0.0f;
            float duration = 0.4f;
            t = t / duration;
            if (t > 1.0f) t = 1.0f;
            float ease = 1.0f - powf(1.0f - t, 3.0f);
            
            if (isWizard) return -S(190.0f) * ease; // slide out left
            if (isSubMenuTransition) {
                bool isGoingBack = (g_sidebarMenu == SidebarMenu::Main);
                if (isGoingBack) {
                    if (isEntering) return -S(190.0f) * (1.0f - ease);
                    else return S(190.0f) * ease;
                } else {
                    if (isEntering) return S(190.0f) * (1.0f - ease);
                    else return -S(190.0f) * ease;
                }
            }
            
            return S(190.0f) * (1.0f - ease);  // slide in from right
        };

        bool drawWizard = (g_app.currentPage == Page::InstallerWizard || g_sidebarAnimState == SidebarAnimState::WizardOut_MenuIn);
        bool drawMenu = (g_app.currentPage != Page::InstallerWizard || g_sidebarAnimState == SidebarAnimState::WizardOut_MenuIn);

        ImVec2 startCursorPos = ImGui::GetCursorPos();

        if (drawWizard) {
            ImGui::SetCursorPos(startCursorPos);
            const char* steps[] = { u8"Приветствие", u8"Запуск", u8"Папка с игрой", u8"Установка", u8"Готово" };
            for (int i = 0; i < 5; ++i) {
                float offset = GetStaggeredOffset(i, true, false, false);
                if (g_app.currentPage != Page::InstallerWizard && g_sidebarAnimState == SidebarAnimState::None) continue; // safety
                ImGui::PushID(i + 100);
                NavButton(steps[i], static_cast<int>(g_wizardStep) == i, offset, false);
                ImGui::PopID();
            }
        }

        if (drawMenu) {
            auto DrawMainMenu = [&](bool isEntering) {
                ImGui::SetCursorPos(startCursorPos);
                ImGui::PushID(200);
                if (NavButton(u8"Главная", g_app.currentPage == Page::Overview, GetStaggeredOffset(0, false, g_sidebarAnimState == SidebarAnimState::SubMenuTransition, isEntering))) SwitchMainPage(Page::Overview);
                if (NavButton(u8"Настройки", g_app.currentPage == Page::Settings, GetStaggeredOffset(1, false, g_sidebarAnimState == SidebarAnimState::SubMenuTransition, isEntering))) SwitchMainPage(Page::Settings);
                if (NavButton(u8"Моды", g_app.currentPage == Page::RtxMods && g_sidebarAnimState == SidebarAnimState::None, GetStaggeredOffset(2, false, g_sidebarAnimState == SidebarAnimState::SubMenuTransition, isEntering))) {
                    g_sidebarMenuPrevious = g_sidebarMenu;
                    g_sidebarMenu = SidebarMenu::RtxGames;
                    g_sidebarAnimState = SidebarAnimState::SubMenuTransition;
                    g_sidebarAnimTimer = 0.0f;
                }
                ImGui::PopID();
            };
            
            auto DrawRtxGamesMenu = [&](bool isEntering) {
                ImGui::SetCursorPos(startCursorPos);
                ImGui::PushID(300);
                if (NavButton(u8"← Назад", false, GetStaggeredOffset(0, false, true, isEntering))) {
                    g_sidebarMenuPrevious = g_sidebarMenu;
                    g_sidebarMenu = SidebarMenu::Main;
                    g_sidebarAnimState = SidebarAnimState::SubMenuTransition;
                    g_sidebarAnimTimer = 0.0f;
                    SwitchMainPage(Page::Overview);
                }
                if (NavButton(u8"Metrostroi RTX", g_app.currentPage == Page::RtxMods, GetStaggeredOffset(1, false, true, isEntering))) {
                    SwitchMainPage(Page::RtxMods);
                }
                ImGui::PopID();
            };

            if (g_sidebarAnimState == SidebarAnimState::SubMenuTransition) {
                if (g_sidebarMenu == SidebarMenu::Main) {
                    DrawRtxGamesMenu(false); // Sliding out
                    DrawMainMenu(true);      // Sliding in
                } else {
                    DrawMainMenu(false);     // Sliding out
                    DrawRtxGamesMenu(true);  // Sliding in
                }
            } else {
                if (g_sidebarMenu == SidebarMenu::Main) DrawMainMenu(true);
                else if (g_sidebarMenu == SidebarMenu::RtxGames) DrawRtxGamesMenu(true);
            }
        }
        
        ImGui::PopStyleVar(); // Pop ItemSpacing
        
        ImGui::SetCursorPos(ImVec2(S(16.0f), ImGui::GetWindowHeight() - S(30.0f)));
        ImGui::PushFont(g_imFontSmall);
        std::string bottomStr = "v" + WStringToUTF8(LauncherUpdater::CURRENT_VERSION) + " (Build " + std::to_string(LauncherUpdater::CURRENT_BUILD_NUMBER) + ")";
        
        ImVec2 textSize = ImGui::CalcTextSize(bottomStr.c_str());
        ImVec2 curPos = ImGui::GetCursorPos();
        
        ImGui::InvisibleButton("VersionBtn", textSize);
        bool isHovered = ImGui::IsItemHovered() && g_app.currentPage != Page::InstallerWizard;
        if (ImGui::IsItemClicked() && g_app.currentPage != Page::InstallerWizard) {
            SwitchMainPage(Page::Authors);
        }
        
        ImGui::SetCursorPos(curPos);
        if (g_app.currentPage == Page::Authors) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.90f, 0.46f, 1.0f));
        } else if (isHovered) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
        }
        ImGui::Text("%s", bottomStr.c_str());
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        ImGui::PopFont();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::EndGroup(); // End Sidebar
        
        ImGui::SameLine(0, 0);
        
        ImGui::BeginGroup(); // Main Content Area
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.06f, 1.0f));
        
        auto RenderPage = [&](Page p) {
            if (p == Page::Overview) RenderUI_Overview();
            else if (p == Page::Updates) RenderUI_Updates();
            else if (p == Page::Settings) RenderUI_Settings();
            else if (p == Page::RtxMods) RenderUI_RtxMods();
            else if (p == Page::Authors) RenderUI_Authors();
            else if (p == Page::InstallerWizard) RenderUI_Wizard();
        };

        if (g_pageIsSliding && g_app.currentPage != Page::InstallerWizard && g_pagePrevious != Page::InstallerWizard) {
            if (g_pageSlideDelayTimer < 0.05f) {
                g_pageSlideDelayTimer += ImGui::GetIO().DeltaTime;
            } else {
                g_pageSlideProgress += ImGui::GetIO().DeltaTime / 0.20f;
            }
            
            if (g_pageSlideProgress >= 1.0f) {
                g_pageSlideProgress = 1.0f;
                g_pageIsSliding = false;
            }
            
            float p = g_pageSlideProgress;
            float easeP = 1.0f - powf(1.0f - p, 3.0f);
            
            float contentW = ImGui::GetContentRegionAvail().x;
            float contentH = ImGui::GetContentRegionAvail().y;
            float slideDist = contentH;
            
            bool slideDown = static_cast<int>(g_pageTarget) > static_cast<int>(g_pagePrevious);
            float oldOffsetY = slideDown ? (-slideDist * easeP) : (slideDist * easeP);
            float newOffsetY = slideDown ? (slideDist * (1.0f - easeP)) : (-slideDist * (1.0f - easeP));

            ImGui::BeginChild("SlideClipBox", ImVec2(contentW, contentH), false, ImGuiWindowFlags_NoScrollbar);
            
            // Old Page
            ImGui::SetCursorPos(ImVec2(0, oldOffsetY));
            ImGui::BeginChild("ContentAreaOld", ImVec2(contentW, contentH), false, ImGuiWindowFlags_NoScrollbar);
            RenderPage(g_pagePrevious);
            ImGui::EndChild();
            
            // New Page
            ImGui::SetCursorPos(ImVec2(0, newOffsetY));
            ImGui::BeginChild("ContentAreaNew", ImVec2(contentW, contentH), false, ImGuiWindowFlags_NoScrollbar);
            RenderPage(g_pageTarget);
            ImGui::EndChild();
            
            ImGui::EndChild(); // SlideClipBox
        } else {
            ImGui::BeginChild("ContentArea", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
            RenderPage(g_app.currentPage);
            ImGui::EndChild(); // ContentArea
        }
        
        ImGui::PopStyleColor(); // ContentArea Bg
        ImGui::EndGroup(); // ContentArea Group
    }
    
    ImGui::PopStyleVar(); // g_uiAlpha
    ImGui::End(); // RTX Launcher main window


    // --- Custom Animated Modal ---
    if (g_diskModalState != DiskModalState::Closed) {
        ImGui::SetNextWindowPos(ImVec2(0, 32.0f));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 32.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.75f * g_diskModalAlpha));
        ImGui::Begin("ModalOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::End();
        ImGui::PopStyleColor();

        if (g_diskModalState == DiskModalState::Opening) {
            g_diskModalTimer += ImGui::GetIO().DeltaTime;
            g_diskModalAlpha = g_diskModalTimer / 0.2f;
            if (g_diskModalAlpha >= 1.0f) { g_diskModalAlpha = 1.0f; g_diskModalState = DiskModalState::Open; }
        }
        else if (g_diskModalState == DiskModalState::Transforming) {
            g_diskModalTimer += ImGui::GetIO().DeltaTime;
            float p = g_diskModalTimer / 0.4f;
            if (p >= 1.0f) { p = 1.0f; g_diskModalState = DiskModalState::ProgressBar; }
        }
        else if (g_diskModalState == DiskModalState::Closing) {
            g_diskModalTimer += ImGui::GetIO().DeltaTime;
            g_diskModalAlpha = 1.0f - (g_diskModalTimer / 0.2f);
            if (g_diskModalAlpha <= 0.0f) { g_diskModalAlpha = 0.0f; g_diskModalState = DiskModalState::Closed; }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_diskModalAlpha);

        float w = 600.0f;
        float h = 280.0f;
        float listAlpha = 1.0f;

        if (g_diskModalState == DiskModalState::Transforming) {
            float p = g_diskModalTimer / 0.4f;
            if (p > 1.0f) p = 1.0f;
            float t = p < 0.5f ? 2.0f * p * p : 1.0f - std::pow(-2.0f * p + 2.0f, 2.0f) / 2.0f;
            h = 280.0f + (140.0f - 280.0f) * t;
            listAlpha = 1.0f - (p * 2.0f);
            if (listAlpha < 0.0f) listAlpha = 0.0f;
        }
        else if (g_diskModalState == DiskModalState::ProgressBar || g_diskModalState == DiskModalState::Closing) {
            h = 140.0f;
            listAlpha = 0.0f;
        }

        ImGui::SetNextWindowSize(ImVec2(w, h));
        ImVec2 clientCenter = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 32.0f + (ImGui::GetIO().DisplaySize.y - 32.0f) * 0.5f);
        ImGui::SetNextWindowPos(clientCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::Begin("DiskSelectionWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

        if (g_diskModalState == DiskModalState::Opening || g_diskModalState == DiskModalState::Open || g_diskModalState == DiskModalState::Transforming) {
            if (listAlpha > 0.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, listAlpha);
                ImGui::PushFont(g_imFontHeading);
                ImGui::SetCursorPos(ImVec2(20, 16));
                ImGui::Text((const char*)u8"Выберите диск для установки");
                ImGui::PopFont();
                ImGui::SetCursorPos(ImVec2(20, 48));
                ImGui::Separator();

                ImGui::SetCursorPos(ImVec2(20, 56));
                ImGui::BeginChild("DiskList", ImVec2(560, 165), true);
                for (size_t i = 0; i < g_app.availableDisks.size(); ++i) {
                    const auto& disk = g_app.availableDisks[i];
                    double freeGb = (double)disk.freeSpace / (1024.0 * 1024.0 * 1024.0);
                    double totalGb = (double)disk.totalSpace / (1024.0 * 1024.0 * 1024.0);
                    std::string utf8_path = WStringToUTF8(disk.path);
                    std::string utf8_name = WStringToUTF8(disk.name);

                    char label[256];
                    if (utf8_name == u8"Локальный диск") {
                        snprintf(label, sizeof(label), "Диск %s##%d", utf8_path.c_str(), (int)i);
                    } else {
                        snprintf(label, sizeof(label), "Диск %s (%s)##%d", utf8_path.c_str(), utf8_name.c_str(), (int)i);
                    }
                    char info[256];
                    snprintf(info, sizeof(info), "%s | Св. %.1f ГБ из %.1f ГБ", disk.isSSD ? "SSD (Рек.)" : "HDD", freeGb, totalGb);

                    bool clicked = ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 36));

                    float nextItemY = ImGui::GetCursorPosY();
                    float infoWidth = ImGui::CalcTextSize(info).x;
                    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - infoWidth - 10);
                    float currentY = ImGui::GetCursorPosY();
                    ImGui::SetCursorPosY(currentY + 8);
                    ImGui::TextDisabled("%s", info);
                    ImGui::SetCursorPosY(nextItemY);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    if (clicked && g_diskModalState == DiskModalState::Open) {
                        g_app.installRootPath = disk.path + L"Metrostroi RTX";
                        SaveSettings();
                        g_diskModalState = DiskModalState::Transforming;
                        g_diskModalTimer = 0.0f;
                        if (g_app.onDiskSelected) {
                            void (*cb)() = g_app.onDiskSelected;
                            g_app.onDiskSelected = nullptr;
                            cb();
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(20, 232));
                if (ImGui::Button((const char*)u8"Отмена", ImVec2(110, 34))) {
                    g_diskModalState = DiskModalState::Closing;
                    g_diskModalTimer = 0.0f;
                    g_app.onDiskSelected = nullptr;
                }
                ImGui::PopStyleVar(); // listAlpha
            }
        }

        if (g_diskModalState == DiskModalState::Transforming || g_diskModalState == DiskModalState::ProgressBar || g_diskModalState == DiskModalState::Closing) {
            float progressAlpha = 1.0f;
            if (g_diskModalState == DiskModalState::Transforming) {
                float p = g_diskModalTimer / 0.4f;
                progressAlpha = (p - 0.5f) * 2.0f;
                if (progressAlpha < 0.0f) progressAlpha = 0.0f;
            }
            if (progressAlpha > 0.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, progressAlpha);

                ImVec4 titleCol = g_app.isFirstLaunchMode ? ImVec4(0.0f, 0.82f, 1.0f, 1.0f) : ImVec4(0.46f, 0.73f, 0.0f, 1.0f);
                ImU32 barCol = g_app.isFirstLaunchMode ? IM_COL32(0, 195, 255, 255) : IM_COL32(118, 185, 0, 255);

                ImGui::SetCursorPos(ImVec2(20, 16));
                ImGui::PushFont(g_imFontHeading);
                std::wstring temp_title;
                {
                    std::lock_guard<std::mutex> lock(g_app.statsMutex);
                    temp_title = g_app.downloadTitleText;
                }
                if (temp_title.empty()) {
                    temp_title = L"Подготовка...";
                }
                std::string utf8_title = WStringToUTF8(temp_title);
                ImGui::PushTextWrapPos(450.0f);
                ImGui::TextColored(titleCol, "%s", utf8_title.c_str());
                ImGui::PopTextWrapPos();
                ImGui::PopFont();

                // Кнопка Отмена
                ImGui::SetCursorPos(ImVec2(480, 14));
                if (ImGui::Button((const char*)u8"Отмена", ImVec2(100, 32))) {
                    DoStop();
                    g_diskModalState = DiskModalState::Closing;
                    g_diskModalTimer = 0.0f;
                }

                // Статистика
                ImGui::SetCursorPos(ImVec2(20, 52));
                std::wstring temp_stats;
                {
                    std::lock_guard<std::mutex> lock(g_app.statsMutex);
                    temp_stats = g_app.downloadStatsText;
                }
                std::string utf8_stats = WStringToUTF8(temp_stats);
                ImGui::PushFont(g_imFontSmall);
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", utf8_stats.c_str());
                ImGui::PopFont();

                // Полоса прогресса 560px
                ImGui::SetCursorPos(ImVec2(20, 84));
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + 560.0f, p0.y + 10.0f);
                drawList->AddRectFilled(p0, p1, IM_COL32(40, 42, 54, 255), 5.0f);

                float fillProgress = g_app.downloadProgressSmooth;
                if (fillProgress < 0.05f) fillProgress = 0.05f;
                if (fillProgress > 1.0f) fillProgress = 1.0f;

                float fw = 560.0f * fillProgress;
                ImVec2 p2 = ImVec2(p0.x + fw, p1.y);
                drawList->AddRectFilled(p0, p2, barCol, 5.0f);

                ImGui::PopStyleVar();
            }
        }

        ImGui::End();
        ImGui::PopStyleVar(); // rounding
        ImGui::PopStyleVar(); // g_diskModalAlpha
    }

    // --- Launch Mode Modal ---
    if (g_app.showLaunchModeModal && g_launchModalState != LaunchModeModalState::Closed) {
        float dt = ImGui::GetIO().DeltaTime;
        g_launchModalTimer += dt;
        float alpha = 1.0f;

        if (g_launchModalState == LaunchModeModalState::Opening) {
            float p = g_launchModalTimer / 0.3f;
            if (p >= 1.0f) { p = 1.0f; g_launchModalState = LaunchModeModalState::Open; g_launchModalTimer = 0.0f; }
            alpha = p;
        }
        else if (g_launchModalState == LaunchModeModalState::Closing) {
            float p = g_launchModalTimer / 0.3f;
            if (p >= 1.0f) { p = 1.0f; g_launchModalState = LaunchModeModalState::Closed; g_app.showLaunchModeModal = false; }
            alpha = 1.0f - p;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.7f * alpha));
        ImGui::Begin("LaunchModeOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::End();
        ImGui::PopStyleColor();

        float w = 600.0f;
        float h = 300.0f;

        if (g_launchModalState == LaunchModeModalState::Opening) {
            float p = g_launchModalTimer / 0.3f;
            p = 1.0f - pow(1.0f - p, 3.0f);
            h = 300.0f * p;
        }
        else if (g_launchModalState == LaunchModeModalState::Closing) {
            float p = g_launchModalTimer / 0.3f;
            p = 1.0f - pow(1.0f - p, 3.0f);
            h = 300.0f * (1.0f - p);
        }

        ImGui::SetNextWindowSize(ImVec2(w, h));
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::Begin("LaunchModeWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

        if (g_launchModalState == LaunchModeModalState::Opening || g_launchModalState == LaunchModeModalState::Open) {
            ImGui::PushFont(g_imFontHeading);
            ImGui::SetCursorPos(ImVec2(20, 20));
            ImGui::Text((const char*)u8"Режим запуска");
            ImGui::PopFont();
            ImGui::SetCursorPos(ImVec2(20, 60));
            ImGui::Separator();

            ImGui::SetCursorPos(ImVec2(20, 80));
            if (ImGui::Button((const char*)u8"Обычный", ImVec2(270, 80))) {
                g_app.launchMode = 1;
                SaveSettings();
                g_launchModalState = LaunchModeModalState::Closing;
                g_launchModalTimer = 0.0f;
                if (g_onLaunchModeSelected) g_onLaunchModeSelected();
            }
            ImGui::SetCursorPos(ImVec2(310, 80));
            if (ImGui::Button((const char*)u8"Режим совместимости", ImVec2(270, 80))) {
                g_app.launchMode = 2;
                SaveSettings();
                g_launchModalState = LaunchModeModalState::Closing;
                g_launchModalTimer = 0.0f;
                if (g_onLaunchModeSelected) g_onLaunchModeSelected();
            }

            ImGui::SetCursorPos(ImVec2(20, 180));
            ImGui::PushFont(g_imFontSmall);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text((const char*)u8"Эти настройки можно будет изменить позже в настройках лаунчера");
            ImGui::PopStyleColor();
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    // --- Changelog Modal ---
    if (g_app.showChangelog) {
        ImGui::OpenPopup(u8"Список изменений");
        g_app.showChangelog = false;
    }
    
    ImGui::SetNextWindowSizeConstraints(ImVec2(S(400), S(200)), ImVec2(S(800), S(600)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(20), S(20)));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, S(10));
    if (ImGui::BeginPopupModal(u8"Список изменений", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextWrapped("%s", WStringToUTF8(g_app.changelogText).c_str());
        ImGui::PopFont();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button(u8"Закрыть", ImVec2(S(120), S(35)))) {
            ImGui::CloseCurrentPopup();
            g_app.changelogText.clear();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

bool CreateDeviceD3D(HWND hWnd) {
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;
    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (g_pSwapChain && SUCCEEDED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))) && pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}





int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    (void)hPrevInstance;
    
    std::wstring cmdLine(lpCmdLine);
    if (cmdLine.find(L"--updated") != std::wstring::npos) {
        wchar_t exePathBuf[MAX_PATH];
        GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH);
        std::wstring changelogPath = std::wstring(exePathBuf);
        size_t lastSlash = changelogPath.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            changelogPath = changelogPath.substr(0, lastSlash + 1) + L"changelog_temp.txt";
        }
        FILE* f = nullptr;
        _wfopen_s(&f, changelogPath.c_str(), L"rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::string utf8Notes(sz, '\0');
            fread(&utf8Notes[0], 1, sz, f);
            fclose(f);
            g_app.changelogText = UTF8ToWString(utf8Notes);
            g_app.showChangelog = true;
            _wremove(changelogPath.c_str());
        }
    }

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    (void)CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    Theme::Init();
    g_app.gpuInfo = DetectGPU();

    std::error_code ec;
    if (!g_app.installRootPath.empty()) {
        fs::create_directories(g_app.installRootPath, ec);
    }

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    const wchar_t* className = L"MetrostroiRtxLauncherWndClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProcImGui;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    RegisterClassW(&wc);

    float dummyW, dummyH;
    CalculateUiScale(dummyW, dummyH);

    int windowWidth = (int)(340.0f * g_uiScale);
    int windowHeight = (int)(140.0f * g_uiScale);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    HWND hwnd = CreateWindowExW(0, className, L"RTX Launcher",
        (WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX),
        x, y, windowWidth, windowHeight,
        nullptr, nullptr, hInstance, nullptr);
    g_app.hMain = hwnd;

    HICON hAppIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    if (hAppIcon) {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    }

    HRGN hInitRgn = CreateRoundRectRgn(0, 0, windowWidth + 1, windowHeight + 1, 24, 24);
    SetWindowRgn(hwnd, hInitRgn, TRUE);

    BOOL dark = TRUE;
    DwmSetWindowAttribute(g_app.hMain, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    if (!CreateDeviceD3D(hwnd)) {

        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    LoadSettings();
    ::ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    AutoDetectGmod();
    // LoadRtxReleases(); // проверка компонентов RTX Remix теперь вызывается только перед запуском игры

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 14.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.09f, 0.10f, 0.13f, 0.85f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.08f, 0.09f, 0.12f, 0.96f);
    colors[ImGuiCol_Border]             = ImVec4(0.18f, 0.22f, 0.28f, 0.40f);
    colors[ImGuiCol_BorderShadow]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.12f, 0.14f, 0.18f, 0.60f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.16f, 0.19f, 0.24f, 0.80f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.20f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.05f, 0.06f, 0.08f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.18f, 0.22f, 0.28f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.29f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    colors[ImGuiCol_CheckMark]          = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.00f, 1.00f, 0.52f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.12f, 0.15f, 0.20f, 0.70f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.18f, 0.23f, 0.30f, 0.90f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.00f, 0.80f, 0.40f, 1.00f);
    colors[ImGuiCol_Header]             = ImVec4(0.12f, 0.15f, 0.20f, 0.60f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.16f, 0.21f, 0.28f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.00f, 0.75f, 0.38f, 1.00f);
    colors[ImGuiCol_Separator]          = ImVec4(0.18f, 0.22f, 0.28f, 0.30f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.00f, 0.90f, 0.46f, 0.78f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_Text]               = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.52f, 0.56f, 0.64f, 1.00f);

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 2;
    g_imFontTitle = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 32.0f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    g_imFontHeading = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 24.0f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    g_imFontRegular = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    g_imFontSmall = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 14.0f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    io.FontDefault = g_imFontRegular;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);



    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        if (g_winAnim.active) {
            g_winAnim.timer += io.DeltaTime;
            float t = g_winAnim.timer / g_winAnim.duration;
            if (t >= 1.0f) {
                t = 1.0f;
                g_winAnim.active = false;
                float targetW, targetH;
                CalculateUiScale(targetW, targetH);
                g_ResizeWidth = (UINT)targetW;
                g_ResizeHeight = (UINT)targetH;
            }
            // Cubic ease out
            float f = 1.0f - powf(1.0f - t, 3.0f);

            int curX = (int)(g_winAnim.startX + (g_winAnim.endX - g_winAnim.startX) * f);
            int curY = (int)(g_winAnim.startY + (g_winAnim.endY - g_winAnim.startY) * f);
            int curW = (int)(g_winAnim.startW + (g_winAnim.endW - g_winAnim.startW) * f);
            int curH = (int)(g_winAnim.startH + (g_winAnim.endH - g_winAnim.startH) * f);

            SetWindowPos(hwnd, nullptr, curX, curY, curW, curH, SWP_NOZORDER);
            if (curW > 0 && curH > 0) {
                HRGN hAnimRgn = CreateRoundRectRgn(0, 0, curW + 1, curH + 1, 24, 24);
                SetWindowRgn(hwnd, hAnimRgn, TRUE);
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();



        RenderImGuiUI();



        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        bool animActive = IsAnimationActive();
        if (animActive) {
            g_pSwapChain->Present(0, 0); // Разблокированная подача кадров для мониторов 144Hz/165Hz/240Hz
        }
        else {
            g_pSwapChain->Present(0, 0);
            Sleep(33); // 30 FPS в статичном состоянии без анимаций (33ms)
        }
    }


    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    Theme::Shutdown();
    CoUninitialize();

    return 0;
}


