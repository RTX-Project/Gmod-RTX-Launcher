#pragma once

// main.cpp - Metrostroi RTX Remix Launcher (Win32, без внешних зависимостей).
//
// Интерфейс оформлен в тёмной теме в духе GOG Galaxy 2.0: сайдбар слева,
// переключаемые страницы (Обзор / Обновления RTX / Steam), консоль лога
// справа снизу, скруглённые кнопки с ховер-эффектом (см. Theme.h).
//
// Собирается в Visual Studio 2026 как "Windows Desktop Application"
// (подсистема Windows, Unicode, /std:c++17). Все нужные библиотеки
// подключены через #pragma comment в заголовках - руками добавлять
// ничего не нужно, кроме самого Windows SDK (ставится вместе с рабочей
// нагрузкой "Разработка классических приложений на C++").

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
#include <shellapi.h>
#include <winioctl.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <fstream>

#include "Theme.h"
#include "FileSync.h"
#include "SteamBeta.h"
#include "RtxRemixUpdater.h"
#include "BinaryPatcher.h"
#include "GameFixesUpdater.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
// Идентификаторы элементов управления
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

enum class Page { Overview = 0, Updates = 1, Settings = 2
#ifdef _DEBUG
    , Developer = 3
#endif
    , UpdateCheck = 4
};

struct DiskInfo {
    std::wstring path;
    std::wstring name;
    uint64_t totalSpace;
    uint64_t freeSpace;
    bool isSSD;
    bool isHDD;
};

// ----------------------------------------------------------------------------
// Глобальное состояние приложения
// ----------------------------------------------------------------------------
struct AppState {
    bool showDownloadPanel = false;
    float downloadPanelAlpha = 0.0f;
    std::wstring downloadStatsText;
    std::vector<std::string> consoleLines;
    std::mutex consoleMutex;
    Page currentPage = Page::Overview;

    std::wstring autoGameSourcePath;
    
    // Download state
    std::atomic<bool> isDownloading{false};
    std::wstring installRootPath;
    bool showDiskModal = false;
    std::vector<DiskInfo> availableDisks;
    HBITMAP modalBgCapture = nullptr;
    void (*onDiskSelected)() = nullptr;

    float downloadProgress = 0.0f;
    float downloadProgressSmooth = 0.0f;
    std::wstring downloadStatsText;
    float downloadPanelAlpha = 0.0f; // 0.0..1.0 для плавного появления
    float downloadPanelOffsetY = 80.0f; // Для анимации выезда снизу
    
    std::wstring repoBinaryPatches = L"BlueAmulet/SourceRTXTweaks";
    std::wstring repoFixes = L"Xenthio/garrys-mod-rtx-remixed";
    
    bool optRepair = true;
#ifdef _DEBUG
#endif
    float navIndicatorY = 110.0f;
    float navIndicatorTargetY = 110.0f;
    float navIndicatorWidthMain = 3.0f;
    float navIndicatorWidthSub = 3.0f;

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
    std::vector<
};

inline AppState g_app;

static const int kSidebarWidth = 75;
static const int kHeaderHeight = 64;

// ----------------------------------------------------------------------------
// Вспомогательные функции UI
// ----------------------------------------------------------------------------

static std::wstring GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthW(hEdit);
    std::wstring buf(len, L'\0');
    if (len > 0) GetWindowTextW(hEdit, &buf[0], len + 1);
    return buf;
}

// ---- Сохранение путей source/dest между запусками (простой текстовый ----
static fs::path GetAppDataRoot() {
    wchar_t* appData = nullptr;
    size_t len = 0;
    _wdupenv_s(&appData, &len, L"APPDATA");
    if (appData) {
        fs::path p = fs::path(appData) / L"rtx-launcher";
        free(appData);
        return p;
    }
    // Фоллбэк, если переменная не найдена
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    return fs::path(exePath).parent_path() / L"rtx-launcher";
}

// файл рядом с .exe - без реестра, без доп. зависимостей) ----
// файл рядом с .exe - без реестра, без доп. зависимостей) ----
static fs::path SettingsFilePath() {
    return GetAppDataRoot() / L"settings" / L"launcher_settings.txt";
}

static void SaveSettings() {
    std::wofstream f(SettingsFilePath(), std::ios::trunc);
    if (!f) return;
    if (!g_app.installRootPath.empty()) {
        f << L"installRootPath=" << g_app.installRootPath << L"\n";
    }
}

static void LoadSettings() {
    std::wifstream f(SettingsFilePath());
    if (!f) return;
    std::wstring line;
    while (std::getline(f, line)) {
        if (line.rfind(L"installRootPath=", 0) == 0) {
            g_app.installRootPath = line.substr(16);
        }
    }
    
    if (!g_app.installRootPath.empty() && g_app.hBtnChangeDisk) {
        
    }
}

static void SetStatus(const std::wstring& text) { g_app.downloadStatsText = text; }

static void AppendLog(const std::wstring& line) { std::lock_guard<std::mutex> lock(g_app.consoleMutex); std::string s(line.begin(), line.end()); g_app.consoleLines.push_back(s); }

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
    
    
    
    
    
    for (auto h : g_app.hListItems) 
    
    
    
    
    
    for (auto h : g_app.hListItems) 
}

static void ShowPage(Page page) {
    if (page == Page::Updates && g_app.currentPage != Page::Updates) {
        if (g_app.rtxReleases.empty()) {
            
        }
    }
    g_app.currentPage = page;
    
    
    
    
    if (g_app.hPageUpdateCheck) 
#ifdef _DEBUG
    
#endif

    if (page == Page::Overview) g_app.navIndicatorTargetY = 110.0f;
    else if (page == Page::Updates) g_app.navIndicatorTargetY = 156.0f;
    else if (page == Page::Settings) g_app.navIndicatorTargetY = 202.0f;
#ifdef _DEBUG
    else if (page == Page::Developer) g_app.navIndicatorTargetY = 248.0f;
#endif
    SetTimer(g_app.hSidebar, 6, 16, nullptr);

    // Force full redraw to clean up artifacts from transparent hidden panels
    

    if (g_app.hNavOverview) 
    if (g_app.hNavUpdates) 
    if (g_app.hNavSettings) 
#ifdef _DEBUG
    if (g_app.hNavDev) 
#endif
}

// ----------------------------------------------------------------------------
// Фоновые операции
// ----------------------------------------------------------------------------

static void RunInBackground(std::function<void()> fn) {
    if (g_app.operationRunning.load()) {
        AppendLog(L"Пожалуйста дождитесь завершения текущей операции (или нажмите «Стоп»).");
        return;
    }
    g_app.stopRequested = false;
    g_app.operationRunning = true;
    SetControlsEnabled(false);
    std::thread([fn]() {
        try {
            fn();
        } catch (const std::exception& e) {
            std::string msg = e.what();
            AppendLog(L"[КРИТИЧЕСКАЯ ОШИБКА] " + std::wstring(msg.begin(), msg.end()));
        } catch (...) {
            AppendLog(L"[КРИТИЧЕСКАЯ ОШИБКА] Неизвестное исключение.");
        }
        g_app.operationRunning = false;
        SetControlsEnabled(true);
    }).detach();
}

static void g_app.showDownloadPanel = false;

static void ShowDiskSelectionModal(void (*callback)() = nullptr);

static std::wstring FormatSizeStr(double mb);

static void DoMoveGame(std::wstring src, std::wstring dst) {
    RunInBackground([src, dst]() {
        AppendLog(L"======================================================================");
        AppendLog(L"Перенос папки игры: " + src + L"  ->  " + dst);

        g_app.isDownloading = true;
        g_app.stopRequested = false;
        g_app.pauseRequested = false;
        g_app.downloadProgress = 0.0f;
        g_app.downloadStatsText = L"Подсчет файлов...";
        

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
                    g_app.downloadStatsText = L"Перенос на паузе...";
                    
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
                } else if (p.is_regular_file(ec)) {
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
                                    swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с | Осталось около %dс", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta);
                                } else {
                                    swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с | Осталось около %dм %dс", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta / 60, eta % 60);
                                }
                            } else {
                                swprintf_s(buf, L"Перенесено: %s/%s | %.1f МБ/с", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed);
                            }
                            
                            g_app.downloadStatsText = buf;
                            if (totalBytes > 0) g_app.downloadProgress = (float)((double)copiedBytes / (double)totalBytes);
                            
                            
                            lastCopied = copiedBytes;
                            lastUpdate = now;
                        }
                    }
                }
            }
        }

        if (g_app.stopRequested.load()) {
            AppendLog(L"Перенос отменен!");
            g_app.downloadStatsText = L"Отменено!";
            g_app.downloadProgress = 1.0f;
            g_app.downloadProgressSmooth = 1.0f;
            
            g_app.isDownloading = false;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            g_app.showDownloadPanel = false;
            
            g_app.installRootPath = src;
            SaveSettings();
            return;
        }

        AppendLog(L"Удаление старой папки...");
        g_app.downloadStatsText = L"Очистка старых файлов...";
        
        fs::remove_all(src, ec);

        AppendLog(L"Перенос успешно завершен!");
        g_app.downloadStatsText = L"Перенос успешно завершен!";
        g_app.downloadProgress = 1.0f;
        g_app.downloadProgressSmooth = 1.0f;
        

        g_app.isDownloading = false;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        g_app.showDownloadPanel = false;
    });
}

static void OnDiskChanged() {
    if (g_app.installRootPath != g_app.oldInstallRootPath && !g_app.oldInstallRootPath.empty()) {
        DoMoveGame(g_app.oldInstallRootPath, g_app.installRootPath);
    }
}

static void DoSync() {
    std::wstring src = g_app.autoGameSourcePath;
    if (g_app.installRootPath.empty()) {
        ShowDiskSelectionModal(DoSync);
        return;
    }
    std::wstring dst = g_app.installRootPath;
    if (src.empty()) {
        AppendLog(L"Исходная папка игры не найдена.");
        return;
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
        g_app.downloadStatsText = L"Подсчет файлов...";
        

        FileSync sync;
        sync.verifyHash = true;
        sync.deleteRemoved = true;
        auto stats = sync.sync(fs::path(src), fs::path(dst), GetAppDataRoot() / L"launcher",
            [](const std::wstring& msg) { AppendLog(msg); },
            []() { return g_app.stopRequested.load(); },
            [](float p, const std::wstring& text) {
                g_app.downloadProgress = p;
                g_app.downloadStatsText = text;
            },
            []() {
                if (g_app.pauseRequested.load()) {
                    g_app.downloadStatsText = L"Синхронизация на паузе...";
                    
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
            
            g_app.downloadStatsText = L"Успешно завершено!";
            g_app.downloadProgress = 1.0f;
            g_app.downloadProgressSmooth = 1.0f;
            
            
            g_app.isDownloading = false;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        
        g_app.showDownloadPanel = false;
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
        } else if (!result.manifestFound) {
            statusText = L"Ошибка: манифест Garry's Mod не найден (игра не установлена).";
            AppendLog(statusText);
        } else if (result.installDir.empty() || result.gamePath.empty()) {
            statusText = L"Ошибка: не удалось определить путь установки из манифеста.";
            AppendLog(statusText);
        } else {
            g_app.autoGameSourcePath = result.gamePath;
            statusText = L"✓ Garry's Mod найден: " + result.gamePath;
            AppendLog(statusText);
        }
        
        
        
    });
}

static void UpdateRtxUI() {
    int selected = -1;
    {
        std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
        selected = g_app.rtxSelectedIndex;
    }
    
    if (selected == 0) {
        
        
        
        
        
        
    } else if (selected == 1) {
        
        
        
        
        
        
    } else {
        
        
        
        
        
         // Прячем и текст, чтобы не было чёрного прямоугольника
    }
    
    // Принудительно перерисовываем родительскую панель, чтобы стереть старый текст (проблема прозрачного фона)
    
}

static void LoadRtxReleases() {
    
}

// Forward declaration - defined after DoApplyRtxVersion
static bool CheckSteamBranch();
static void ShowDiskSelectionModal();

static std::wstring FormatSizeStr(double mb) {
    wchar_t buf[64];
    if (mb >= 1000.0) {
        swprintf_s(buf, L"%.1f ГБ", mb / 1024.0);
    } else {
        swprintf_s(buf, L"%.1f МБ", mb);
    }
    return buf;
}

static void DoApplyRtxVersion() {
    int selected = -1;
    {
        std::lock_guard<std::mutex> lock(g_app.lastCheckMutex);
        selected = g_app.rtxSelectedIndex;
    }
    if (selected < 0 || selected > 1) {
        AppendLog(L"Режим не выбран.");
        return;
    }

    std::wstring workingPath = g_app.installRootPath;
    if (workingPath.empty()) {
        ShowDiskSelectionModal(DoApplyRtxVersion);
        return;
    }
    if (!fs::exists(workingPath)) {
        AppendLog(L"Рабочая папка игры не существует. Запустите первоначальную установку.");
        return;
    }

    RunInBackground([selected, workingPath]() {
        AppendLog(L"====================================================================");

        // Проверка бета-ветки перед скачиванием
        AppendLog(L"Проверка бета-ветки Steam перед запуском...");
        if (!CheckSteamBranch()) {
            AppendLog(L"[ПРЕДУПРЕЖДЕНИЕ] Продолжаем установку мода, но игра может работать некорректно.");
        }

        
        std::wstring rtxRemixPath = workingPath + L"\\rtx-remix";
        std::wstring modsPath = rtxRemixPath + L"\\mods";
        std::wstring disablePath = rtxRemixPath + L"\\disabled_mods";

        std::error_code ec;
        fs::create_directories(modsPath, ec);
        fs::create_directories(disablePath, ec);

        std::wstring targetFolder = (selected == 0) ? L"lighting_only" : L"lighting_and_textures";
        std::wstring otherFolder = (selected == 0) ? L"lighting_and_textures" : L"lighting_only";
        
        // ВАЖНО: Замените эти ссылки на прямые ссылки на ZIP архивы на ModDB
        std::wstring targetUrl = (selected == 0) ? 
            L"https://github.com/Xenthio/garrys-mod-rtx-remixed/releases/download/v0.1.0/LightingOnly_Placeholder.zip" : 
            L"https://github.com/Xenthio/garrys-mod-rtx-remixed/releases/download/v0.1.0/LightingTextures_Placeholder.zip";

        AppendLog(L"Применение мода: " + targetFolder + L"...");

        // Отключение старого мода
        if (fs::exists(modsPath + L"\\" + otherFolder, ec)) {
            AppendLog(L"Отключение старой версии мода (" + otherFolder + L")...");
            fs::rename(modsPath + L"\\" + otherFolder, disablePath + L"\\" + otherFolder, ec);
            if (ec) AppendLog(L"Ошибка отключения: " + ([](const std::string& s) { return std::wstring(s.begin(), s.end()); })(ec.message()));
        }

        // Включение нужного мода
        if (fs::exists(modsPath + L"\\" + targetFolder, ec)) {
            AppendLog(L"Мод уже установлен и активен.");
            return;
        }

        if (fs::exists(disablePath + L"\\" + targetFolder, ec)) {
            AppendLog(L"Включение мода из папки disabled_mods...");
            fs::rename(disablePath + L"\\" + targetFolder, modsPath + L"\\" + targetFolder, ec);
            if (ec) {
                AppendLog(L"Ошибка включения: " + ([](const std::string& s) { return std::wstring(s.begin(), s.end()); })(ec.message()));
            } else {
                AppendLog(L"Мод успешно применен.");
            }
            return;
        }

        // Скачивание
        try {
            AppendLog(L"Мод не найден локально. Начинаю загрузку архива...");
            std::wstring tempZip = (GetAppDataRoot() / L"temp_mod.zip").wstring();
            
            g_app.isDownloading = true;
            g_app.stopRequested = false;
            g_app.pauseRequested = false;
            g_app.downloadProgress = 0.0f;
            g_app.downloadStatsText = L"Подключение...";
             // Вызываем обновление UI из фонового потока
             // триггер ShowDownloadPanelFade
            
            auto startTime = std::chrono::steady_clock::now();
            auto lastUpdate = startTime;
            double pausedSeconds = 0.0;
            uint64_t lastDownloaded = 0;
            double currentSpeed = 0.0;
            
            HttpClient::downloadFile(targetUrl, tempZip, L"RtxLauncher/1.0", [&](uint64_t downloaded, uint64_t total) {
                if (g_app.stopRequested.load()) return false;
                
                if (g_app.pauseRequested.load()) {
                    g_app.downloadStatsText = L"Загрузка на паузе...";
                     // Обновить UI
                    auto pauseStart = std::chrono::steady_clock::now();
                    while (g_app.pauseRequested.load()) {
                        if (g_app.stopRequested.load()) return false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    auto pauseEnd = std::chrono::steady_clock::now();
                    pausedSeconds += std::chrono::duration<double>(pauseEnd - pauseStart).count();
                    lastUpdate = pauseEnd;
                    lastDownloaded = downloaded;
                }
                
                auto now = std::chrono::steady_clock::now();
                auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
                
                // Обновляем UI каждые 100мс
                if (dt >= 100 || downloaded == total) {
                    double stepDt = dt / 1000.0;
                    if (stepDt > 0) {
                        double instantSpeed = ((downloaded - lastDownloaded) / 1024.0 / 1024.0) / stepDt;
                        if (currentSpeed == 0.0) currentSpeed = instantSpeed;
                        else currentSpeed = currentSpeed * 0.95 + instantSpeed * 0.05;
                    }
                    double speed = currentSpeed;
                    lastDownloaded = downloaded;
                    lastUpdate = now;
                    
                    double totalMb = total / 1024.0 / 1024.0;
                    double downMb = downloaded / 1024.0 / 1024.0;
                    
                    int eta = -1;
                    if (speed > 0 && total > 0) {
                        eta = (int)((totalMb - downMb) / speed);
                    }
                    
                    wchar_t buf[256];
                    if (total > 0) {
                        g_app.downloadProgress = (float)downloaded / total;
                        if (eta >= 0) {
                            if (eta <= 59) {
                                swprintf_s(buf, L"Скачано: %s/%s | %.1f МБ/с | Осталось около %dс", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta);
                            } else {
                                swprintf_s(buf, L"Скачано: %s/%s | %.1f МБ/с | Осталось около %dм %dс", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed, eta / 60, eta % 60);
                            }
                        } else {
                            swprintf_s(buf, L"Скачано: %s/%s | %.1f МБ/с", FormatSizeStr(downMb).c_str(), FormatSizeStr(totalMb).c_str(), speed);
                        }
                    } else {
                        g_app.downloadProgress = 1.0f; // Indeterminate basically, or we can just draw full
                        swprintf_s(buf, L"Скачано: %s | %.1f МБ/с", FormatSizeStr(downMb).c_str(), speed);
                    }
                    
                    g_app.downloadStatsText = buf;
                    
                    lastUpdate = now;
                }
                return true;
            });

            g_app.isDownloading = false;
            g_app.showDownloadPanel = false;

            if (g_app.stopRequested.load()) {
                AppendLog(L"Скачивание отменено.");
                fs::remove(tempZip, ec);
                
                return;
            }

            AppendLog(L"Архив скачан. Распаковка в " + targetFolder + L"...");
            std::wstring extractPath = modsPath + L"\\" + targetFolder;
            fs::create_directories(extractPath, ec);
            
            ZipExtract::extractAll(tempZip, extractPath);
            fs::remove(tempZip, ec);
            
            AppendLog(L"Мод успешно установлен и применен!");
            g_app.showDownloadPanel = false;
        } catch (const std::exception& e) {
            g_app.isDownloading = false;
            g_app.showDownloadPanel = false;
            std::string msg = e.what();
            AppendLog(L"Ошибка скачивания/установки: " + std::wstring(msg.begin(), msg.end()));
        }
    });
}

static void DoOpenRtxGithub() {
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
            msg += L" Убедитесь, что в свойствах игры выбрана правильная бета-ветка.";
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

static void DoLaunchGame();

static void CloseDiskSelectionModal(HWND hwnd) {
    g_app.showDiskModal = false;
    DestroyWindow(hwnd);
    g_app.hModalOverlay = nullptr;
    
    if (g_app.onDiskSelected) {
        void (*cb)() = g_app.onDiskSelected;
        g_app.onDiskSelected = nullptr;
        cb();
    }
}

static LRESULT CALLBACK DiskModalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_LBUTTONDOWN) {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        RECT rc; GetWindowRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        int boxW = 600, boxH = 400;
        int boxX = (w - boxW) / 2;
        int boxY = (h - boxH) / 2;
        int listY = boxY + 80;
        
        for (size_t i = 0; i < g_app.availableDisks.size(); ++i) {
            int itemY = listY + (int)i * 50;
            if (x >= boxX + 30 && x <= boxX + boxW - 30 && y >= itemY && y <= itemY + 45) {
                g_app.installRootPath = g_app.availableDisks[i].path + L"Metrostroi RTX";
                SaveSettings();
                CloseDiskSelectionModal(hwnd);
                if (g_app.hBtnChangeDisk) {
                    
                }
                break;
            }
        }
        return 0;
    }
    if (msg == WM_DESTROY) {
        if (g_app.showDiskModal) {
            g_app.showDiskModal = false;
            g_app.hModalOverlay = nullptr;
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void ShowDiskSelectionModal(void (*callback)()) {
    if (g_app.hModalOverlay) return;
    g_app.onDiskSelected = callback;
    g_app.availableDisks = GetAvailableDisks();
    g_app.showDiskModal = true;

    static bool s_classReg = false;
    if (!s_classReg) {
        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
        wc.lpfnWndProc = DiskModalWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"DiskModalClass";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExW(&wc);
        s_classReg = true;
    }

    RECT rcMain;
    GetWindowRect(g_app.hMain, &rcMain); 
    int w = rcMain.right - rcMain.left;
    int h = (rcMain.bottom - rcMain.top) - 30;

    g_app.hModalOverlay = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST, 
        L"DiskModalClass", L"", 
        WS_POPUP | WS_VISIBLE, 
        rcMain.left, rcMain.top + 30, w, h, 
        g_app.hMain, nullptr, GetModuleHandle(nullptr), nullptr);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits;
    HBITMAP hBmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

    {
        Graphics g(hdcMem);
        g.SetSmoothingMode(SmoothingModeAntiAlias);

        SolidBrush dimBrush(Color(180, 0, 0, 0));
        g.FillRectangle(&dimBrush, 0, 0, w, h);

        int boxW = 600, boxH = 400;
        int boxX = (w - boxW) / 2;
        int boxY = (h - boxH) / 2;

        GraphicsPath path;
        RectF boxR((float)boxX, (float)boxY, (float)boxW, (float)boxH);
        Theme::RoundedRectPath(path, boxR, 10.0f);
        SolidBrush boxBrush(Color(255, 30, 31, 42));
        g.FillPath(&boxBrush, &path);
        
        Font titleFont(L"Segoe UI", 24, FontStyleBold, UnitPixel);
        SolidBrush textBrush(Color(255, 255, 255, 255));
        StringFormat format; format.SetAlignment(StringAlignmentCenter);
        RectF titleR((float)boxX, (float)boxY + 20.0f, (float)boxW, 30.0f);
        g.DrawString(L"Выберите диск для установки", -1, &titleFont, titleR, &format, &textBrush);

        int listY = boxY + 80;
        Font diskFont(L"Segoe UI", 16, FontStyleBold, UnitPixel);
        Font typeFont(L"Segoe UI", 14, FontStyleRegular, UnitPixel);
        SolidBrush dimTextBrush(Color(120, 255, 255, 255));
        SolidBrush accentBrush(Theme::CurrentAccent());

        for (size_t i = 0; i < g_app.availableDisks.size(); ++i) {
            const auto& disk = g_app.availableDisks[i];
            int itemY = listY + (int)i * 50;
            RectF itemR((float)boxX + 30.0f, (float)itemY, (float)boxW - 60.0f, 40.0f);
            
            StringFormat leftFormat; leftFormat.SetAlignment(StringAlignmentNear);
            leftFormat.SetLineAlignment(StringAlignmentCenter);
            std::wstring diskLabel = disk.path + L" (" + disk.name + L")";
            g.DrawString(diskLabel.c_str(), -1, &diskFont, itemR, &leftFormat, &textBrush);

            StringFormat rightFormat; rightFormat.SetAlignment(StringAlignmentFar);
            rightFormat.SetLineAlignment(StringAlignmentCenter);
            
            double freeGb = (double)disk.freeSpace / (1024.0 * 1024.0 * 1024.0);
            double totalGb = (double)disk.totalSpace / (1024.0 * 1024.0 * 1024.0);
            
            wchar_t spaceBuf[256];
            swprintf_s(spaceBuf, L"%s | Свободно %.1f ГБ из %.1f ГБ", 
                disk.isSSD ? L"SSD (Рекомендуется)" : L"HDD", freeGb, totalGb);

            SolidBrush* pTypeBrush = disk.isSSD ? &accentBrush : &dimTextBrush;
            g.DrawString(spaceBuf, -1, &typeFont, itemR, &rightFormat, pTypeBrush);
            
            Pen linePen(Color(50, 255, 255, 255), 1.0f);
            g.DrawLine(&linePen, boxX + 30, itemY + 45, boxX + boxW - 30, itemY + 45);
        }
    }

    POINT ptSrc = {0, 0};
    POINT ptPos = {rcMain.left, rcMain.top};
    SIZE size = {w, h};
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    
    UpdateLayeredWindow(g_app.hModalOverlay, hdcScreen, &ptPos, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

// Запуск игры с проверкой ветки
static void DoLaunchGame() {
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

    RunInBackground([srcPath, dstPath]() {
        AppendLog(L"====================================================================");
        AppendLog(L"Проверка бета-ветки Steam перед запуском...");
        bool branchOk = CheckSteamBranch();

        if (!branchOk) {
            AppendLog(L"Запуск отменён: неправильная бета-ветка.");
            return;
        }

        AppendLog(L"Быстрая проверка целостности файлов...");
        g_app.isDownloading = true;
        g_app.downloadProgress = 0.0f;
        g_app.downloadStatsText = L"Подготовка файлов перед установкой...";
        

        FileSync sync;
        sync.verifyHash = false; // Быстрая проверка только по размеру/дате
        sync.deleteRemoved = true;
        auto stats = sync.sync(fs::path(srcPath), fs::path(dstPath), GetAppDataRoot() / L"launcher",
            [](const std::wstring& msg) { AppendLog(msg); },
            []() { return g_app.stopRequested.load(); },
            [](float p, const std::wstring& text) {
                g_app.downloadProgress = p;
                g_app.downloadStatsText = text;
            },
            []() {
                if (g_app.pauseRequested.load()) {
                    g_app.downloadStatsText = L"Запуск на паузе...";
                    
                    while (g_app.pauseRequested.load()) {
                        if (g_app.stopRequested.load()) break;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
            });
            g_app.downloadStatsText = L"Установка отменена!";
            g_app.downloadProgress = 1.0f;
            g_app.downloadProgressSmooth = 1.0f;
            
            
            g_app.isDownloading = false;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
        g_app.showDownloadPanel = false;

        if (g_app.stopRequested.load()) {
            AppendLog(L"Запуск отменён.");
            return;
        }

        // Ищем gmod.exe / hl2.exe в папке ГДЕ ЛАУНЧЕР (dstPath)
        std::wstring exePath;
        for (auto& name : { std::wstring(L"gmod.exe"), std::wstring(L"hl2.exe") }) {
            std::wstring candidate = dstPath + L"\\" + name;
            if (fs::exists(candidate)) { exePath = candidate; break; }
        }

        if (exePath.empty()) {
            AppendLog(L"gmod.exe не найден в папке игры. Ошибка конфигурации.");
            return;
        }

        AppendLog(L"Запуск: " + exePath);
        SHELLEXECUTEINFOW sei = {};
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"open";
        sei.lpFile = exePath.c_str();
        sei.lpDirectory = dstPath.c_str();
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExW(&sei);
    });
}

static void DoStop() {
    g_app.stopRequested = true;
    g_app.pauseRequested = false;
    AppendLog(L"Запрошена остановка текущей операции...");
}

// Плавное появление панели загрузки (альфа 0->1.0 за ~180мс через AlphaBlend)
static void ShowDownloadPanelFade() {
    g_app.downloadPanelAlpha = 0.0f;
    g_app.downloadPanelOffsetY = 80.0f;
    g_app.downloadProgressSmooth = 0.0f;
    
    RECT rcClient; GetClientRect(g_app.hMain, &rcClient);
    int h = rcClient.bottom;
    int contentW = rcClient.right - 250; // kSidebarWidth
    SetWindowPos(g_app.hDownloadPanel, nullptr, 250 + 24, h - 160 + (int)g_app.downloadPanelOffsetY, contentW - 48, 72, SWP_NOZORDER);

    
    SetTimer(g_app.hMain, 3, 16, nullptr); // ~60 fps fade
    SetTimer(g_app.hMain, 4, 16, nullptr); // ~60 fps progress bar smooth
}

static void HideDownloadPanelFade() {
    KillTimer(g_app.hMain, 3); // Stop fade in if it's running
    SetTimer(g_app.hMain, 5, 16, nullptr); // ~60 fps fade out
}

// ----------------------------------------------------------------------------
// Создание элементов управления
// ----------------------------------------------------------------------------

