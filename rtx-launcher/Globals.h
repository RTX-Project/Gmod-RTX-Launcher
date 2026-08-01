#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <d3d11.h>
#include "imgui.h"
#include "LauncherUpdater.h"
#include "RtxRemixUpdater.h"

enum class Page {
    Overview = 0, Updates = 1, Settings = 2
#ifdef _DEBUG
    , Developer = 3
#endif
    , UpdateCheck = 4, InstallerWizard = 5, Authors = 6, RtxMods = 7
};

enum class WizardStep { Welcome = 0, LaunchMode = 1, DriveSelect = 2, Progress = 3, Complete = 4 };

enum class RtxModsView { GamesList = 0, ModsList = 1 };


struct DiskInfo {
    std::wstring path;
    std::wstring name;
    uint64_t totalSpace = 0;
    uint64_t freeSpace = 0;
    bool isSSD = false;
    bool isHDD = false;
};

enum class LaunchModeModalState { Closed, Opening, Open, Closing };
enum class DiskModalState { Closed, Opening, Open, Transforming, ProgressBar, Closing };
enum class SidebarAnimState { None, WizardOut_MenuIn, SubMenuTransition };
enum class SidebarMenu { Main, RtxGames };

struct GpuInfo {
    std::string name;
    UINT vendorId = 0;
    UINT deviceId = 0;
    bool isNvidiaRtx = false;
    bool isAmdOrIntel = false;
    bool isGtxOrOlder = false;
    std::string compatibilityNotice;
};

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

// Global application state
extern AppState g_app;
extern LauncherUpdater::UpdateInfo g_launcherUpdateInfo;
extern std::atomic<bool> g_isCheckingLauncherUpdate;
extern WizardStep g_wizardStep;
extern WizardStep g_wizardTargetStep;
extern bool g_wizardIsSliding;
extern float g_wizardSlideProgress;

// Main page transition variables
extern bool g_pageIsSliding;
extern float g_pageSlideProgress;
extern float g_pageSlideDelayTimer;
extern Page g_pageTarget;
extern Page g_pagePrevious;

extern RtxModsView g_rtxModsView;
extern RtxModsView g_rtxModsPreviousView;
extern bool g_rtxModsIsSliding;
extern float g_rtxModsSlideProgress;
extern float g_rtxModsSlideDelayTimer;

extern SidebarMenu g_sidebarMenu;
extern SidebarMenu g_sidebarMenuPrevious;

extern bool g_autoStartGameAfterInstall;
extern LaunchModeModalState g_launchModalState;
extern float g_launchModalTimer;
extern void (*g_onLaunchModeSelected)();
extern DiskModalState g_diskModalState;
extern float g_diskModalTimer;
extern float g_diskModalAlpha;
extern SidebarAnimState g_sidebarAnimState;
extern float g_sidebarAnimTimer;
extern void LoadRtxReleases();
extern void ShowDiskSelectionModal();
extern void CheckLauncherUpdatesAsync();
extern void RenderImGuiUI();

extern ImFont* g_imFontTitle;
extern ImFont* g_imFontHeading;
extern ImFont* g_imFontRegular;
extern ImFont* g_imFontSmall;

extern float g_uiScale;
extern float g_uiAlpha;
extern float g_pageTransitionAlpha;
extern bool g_isPageTransitioning;
extern bool g_isSmoothScrolling;

// Useful helper functions
extern std::string WStringToUTF8(const std::wstring& wstr);
extern void SaveSettings();
extern void GoToWizardStep(WizardStep newStep);
extern void SwitchPage(Page page);
extern void PushAccentButton();
extern void PopAccentButton();
extern void DoLaunchGame();
inline float S(float v) { return v * g_uiScale; }
inline ImVec2 S(float x, float y) { return ImVec2(x * g_uiScale, y * g_uiScale); }
extern std::vector<DiskInfo> GetAvailableDisks();
extern ImVec4 GetAdaptiveProgressColor(float progress);
