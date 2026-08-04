// SteamBeta.h - поиск установки Steam через реестр, поиск библиотек через
// libraryfolders.vdf, чтение appmanifest_<appid>.acf и проверка выбранной
// бета-ветки. Никаких изменений в файлы Steam не вносится - только чтение.
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

// Структура, описывающая результат проверки состояния Steam-игры
struct BetaCheckResult {
    bool steamFound = false;        // Установлен ли Steam на компьютере
    bool manifestFound = false;     // Найден ли манифест игры (appmanifest_*.acf)
    std::wstring manifestPath;      // Полный путь к найденному манифесту
    std::wstring betaKey;           // Текущая бета-ветка, прочитанная из манифеста
    bool onExpectedBranch = false;  // Совпадает ли ветка с ожидаемой
    std::wstring installDir;        // Имя папки с игрой (например "GarrysMod")
    std::wstring gamePath;          // Полный путь к папке с игрой
};

class SteamBeta {
public:
    std::wstring appId = L"4000";           // AppID игры в Steam (по умолчанию Garry's Mod)
    std::wstring expectedBranch = L"x86-64"; // Требуемая бета-ветка для корректной работы RTX

    // Главный метод: ищет Steam, парсит пути библиотек и читает манифест игры,
    // чтобы определить, выбрана ли правильная бета-ветка.
    BetaCheckResult check() {
        BetaCheckResult result;

        std::wstring steamPath = readSteamPathFromRegistry();
        if (steamPath.empty()) {
            return result; // steamFound = false
        }
        result.steamFound = true;

        std::vector<fs::path> libraries;
        libraries.push_back(fs::path(steamPath));

        fs::path vdfPath = fs::path(steamPath) / L"steamapps" / L"libraryfolders.vdf";
        std::wifstream f(vdfPath);
        if (f) {
            std::wstringstream ss;
            ss << f.rdbuf();
            std::wstring content = ss.str();
            std::wregex pathRe(LR"regex("path"\s*"([^"]+)")regex");
            for (std::wsregex_iterator it(content.begin(), content.end(), pathRe), end; it != end; ++it) {
                std::wstring p = (*it)[1].str();
                std::wstring fixed;
                for (size_t i = 0; i < p.size(); ++i) {
                    if (p[i] == L'\\' && i + 1 < p.size() && p[i + 1] == L'\\') { fixed += L'\\'; ++i; }
                    else fixed += p[i];
                }
                libraries.push_back(fs::path(fixed));
            }
        }

        for (auto& lib : libraries) {
            fs::path manifestPath = lib / L"steamapps" / (L"appmanifest_" + appId + L".acf");
            std::error_code ec;
            if (fs::is_regular_file(manifestPath, ec)) {
                result.manifestFound = true;
                result.manifestPath = manifestPath.wstring();

                std::wifstream mf(manifestPath);
                std::wstringstream mss;
                mss << mf.rdbuf();
                std::wstring content = mss.str();

                std::wregex betaRe(LR"regex("BetaKey"\s*"([^"]*)")regex");
                std::wsmatch m;
                if (std::regex_search(content, m, betaRe)) {
                    result.betaKey = m[1].str();
                }
                
                std::wregex installdirRe(LR"regex("installdir"\s*"([^"]*)")regex");
                if (std::regex_search(content, m, installdirRe)) {
                    result.installDir = m[1].str();
                    result.gamePath = (lib / L"steamapps" / L"common" / result.installDir).wstring();
                }

                result.onExpectedBranch = (result.betaKey == expectedBranch);
                return result;
            }
        }

        return result; // manifestFound остаётся false
    }

    // Публичный доступ к пути Steam для передачи в аргументы запуска игры
    std::wstring readSteamPathFromRegistryPublic() {
        return readSteamPathFromRegistry();
    }

private:
    std::wstring readSteamPathFromRegistry() {
        // Сначала пользовательский путь (актуальная установка steam.exe),
        // затем - для 32-бит процесса на 64-бит системе - Wow6432Node.
        const wchar_t* keys[] = {
            L"Software\\Valve\\Steam",
        };
        for (auto keyPath : keys) {
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t buf[MAX_PATH] = {};
                DWORD size = sizeof(buf);
                DWORD type = 0;
                if (RegQueryValueExW(hKey, L"SteamPath", nullptr, &type, (LPBYTE)buf, &size) == ERROR_SUCCESS) {
                    RegCloseKey(hKey);
                    return std::wstring(buf);
                }
                RegCloseKey(hKey);
            }
        }
        // Fallback: HKLM InstallPath
        const wchar_t* hklmKeys[] = {
            L"SOFTWARE\\WOW6432Node\\Valve\\Steam",
            L"SOFTWARE\\Valve\\Steam",
        };
        for (auto keyPath : hklmKeys) {
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t buf[MAX_PATH] = {};
                DWORD size = sizeof(buf);
                DWORD type = 0;
                if (RegQueryValueExW(hKey, L"InstallPath", nullptr, &type, (LPBYTE)buf, &size) == ERROR_SUCCESS) {
                    RegCloseKey(hKey);
                    return std::wstring(buf);
                }
                RegCloseKey(hKey);
            }
        }
        return L"";
    }
};
