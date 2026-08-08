// LauncherUpdater.h - Модуль автоматической проверки и установки обновлений лаунчера
// Репозиторий: https://github.com/RTX-Project/Gmod-RTX-Launcher
#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <filesystem>
#include "HttpClient.h"
#include "JsonValue.h"

extern std::string WStringToUTF8(const std::wstring& wstr);

class LauncherUpdater {
public:



    inline static const std::wstring CURRENT_VERSION = L"0.0.2.7";
    inline static const int CURRENT_BUILD_NUMBER = 285; // Full localization to English
    inline static const long long CURRENT_RELEASE_ID = 0;
    inline static const std::wstring REPO_OWNER = L"RTX-Project";
    inline static const std::wstring REPO_NAME = L"Gmod-RTX-Launcher";

    struct UpdateInfo {
        bool hasUpdate = false;
        bool isMicroUpdate = false;
        long long releaseId = 0;
        int buildNumber = 0;
        std::wstring version;
        std::wstring publishedAt;
        std::wstring downloadUrl;
        std::wstring releaseNotes;
    };

    inline static bool INCLUDE_PRERELEASES = true;

    // Проверка обновлений на GitHub
    static UpdateInfo CheckForUpdate(const std::wstring& authToken = L"", const std::string& targetAsset = "system_data.bin") {
        UpdateInfo info;
        try {
            std::wstring url = INCLUDE_PRERELEASES ?
                (L"https://api.github.com/repos/" + REPO_OWNER + L"/" + REPO_NAME + L"/releases") :
                (L"https://api.github.com/repos/" + REPO_OWNER + L"/" + REPO_NAME + L"/releases/latest");
            
            // Если включены пре-релизы (или пользовательская ссылка)
            std::string jsonStr = HttpClient::getText(url, L"RTX-Launcher-Updater", authToken);
            if (jsonStr.empty()) return info;

            JsonValue rootJson = JsonValue::parse(jsonStr);
            JsonValue targetRelease;

            if (rootJson.isArray()) {
                if (rootJson.arrayValue.empty()) return info;
                int maxBuild = -1;
                std::wstring maxVersionStr = L"";

                for (const auto& rel : rootJson.arrayValue) {
                    std::string tName = rel["tag_name"].asString();
                    std::string rName = rel["name"].asString();
                    int buildNum = ParseBuildNumber(tName, rName);
                    
                    std::wstring relVer = UTF8ToWString(tName);
                    if (relVer.rfind(L"v", 0) == 0 || relVer.rfind(L"V", 0) == 0) {
                        relVer = relVer.substr(1);
                    }

                    if (buildNum > maxBuild) {
                        maxBuild = buildNum;
                        maxVersionStr = relVer;
                        targetRelease = rel;
                    } else if (buildNum == maxBuild) {
                        if (IsVersionNewer(relVer, maxVersionStr)) {
                            maxVersionStr = relVer;
                            targetRelease = rel;
                        }
                    }
                }
                // Fallback, if no build numbers were found (which shouldn't happen), it will use the first one that was parsed (or index 0).
                if (targetRelease.isNull()) targetRelease = rootJson.arrayValue[0];
            } else if (rootJson.isObject()) {
                targetRelease = rootJson;
            } else {
                return info;
            }

            std::string tagNameUtf8 = targetRelease["tag_name"].asString();
            std::string relNameUtf8 = targetRelease["name"].asString();
            std::wstring latestVersion = UTF8ToWString(tagNameUtf8);
            if (latestVersion.rfind(L"v", 0) == 0 || latestVersion.rfind(L"V", 0) == 0) {
                latestVersion = latestVersion.substr(1);
            }

            long long releaseId = targetRelease["id"].asInt64(0);
            std::string pubAtUtf8 = targetRelease["published_at"].asString();
            std::wstring publishedAt = UTF8ToWString(pubAtUtf8);
            int remoteBuild = ParseBuildNumber(tagNameUtf8, relNameUtf8);

            info.version = latestVersion;
            info.releaseId = releaseId;
            info.publishedAt = publishedAt;
            info.buildNumber = remoteBuild;
            
            bool isNewerVersion = IsVersionNewer(latestVersion, CURRENT_VERSION);
            bool isNewerBuild = (remoteBuild > CURRENT_BUILD_NUMBER);
            
            // Если версия выше ИЛИ выложен свежий билд
            if (isNewerVersion || isNewerBuild) {
                info.hasUpdate = true;
                info.isMicroUpdate = (!isNewerVersion && isNewerBuild);
            }

            // Извлекаем дистрибутивы и описания изменений
            if (targetRelease.has("assets") && targetRelease["assets"].isArray()) {
                const auto& assets = targetRelease["assets"].arrayValue;
                for (size_t i = 0; i < assets.size(); ++i) {
                    std::string assetName = assets[i]["name"].asString();
                    if (assetName == targetAsset) {
                        info.downloadUrl = UTF8ToWString(assets[i]["browser_download_url"].asString());
                        break; 
                    }
                }
            }

            // Резервный URL, если в массиве ассетов ничего не найдено, но тег существует
            if (info.downloadUrl.empty() && !tagNameUtf8.empty()) {
                info.downloadUrl = L"https://github.com/" + REPO_OWNER + L"/" + REPO_NAME + L"/releases/download/" + UTF8ToWString(tagNameUtf8) + L"/system_data.bin";
            }
            
            std::string body = targetRelease["body"].asString();
            info.releaseNotes = UTF8ToWString(body);
        }
        catch (const std::exception& ex) {
            info.releaseNotes = L"Ошибка проверки обновлений: " + UTF8ToWString(ex.what());
        }
        catch (...) {
            info.releaseNotes = L"Неизвестная ошибка при подключении к GitHub API.";
        }
        return info;
    }

    // Автоматическое скачивание и бесшовная замена текущего .exe файла
    static bool DownloadAndApplyUpdate(const std::wstring& downloadUrl,
                                        const std::wstring& releaseNotes,
                                        std::function<void(const std::wstring&)> logCb,
                                        std::function<void(float)> progressCb,
                                        const std::wstring& authToken = L"") {
        try {
            if (downloadUrl.empty()) {
                logCb(L"[Updater] Ошибка: Ссылка для скачивания обновления пуста.");
                return false;
            }

            wchar_t exePathBuf[MAX_PATH];
            GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH);
            std::wstring currentExePath = exePathBuf;

            wchar_t tempDirBuf[MAX_PATH];
            GetTempPathW(MAX_PATH, tempDirBuf);
            std::wstring tempExePath = std::wstring(tempDirBuf) + L"rtx_launcher_new.exe";

            logCb(L"[Updater] Скачивание обновления с: " + downloadUrl);

            HttpClient::downloadFile(downloadUrl, tempExePath, L"RTX-Launcher-Updater",
                [progressCb](uint64_t downloaded, uint64_t total) -> bool {
                    if (total > 0) {
                        progressCb((float)downloaded / (float)total);
                    }
                    return true;
                }, authToken);

            logCb(L"[Updater] Распаковка нового билда...");
            logCb(L"[Updater] Готовим скрипт для перезапуска лаунчера...");

            // Сохраняем чейнджлог во временный файл
            std::wstring changelogPath = std::wstring(exePathBuf);
            size_t lastSlash = changelogPath.find_last_of(L"\\/");
            if (lastSlash != std::wstring::npos) {
                changelogPath = changelogPath.substr(0, lastSlash + 1) + L"changelog_temp.txt";
            }
            FILE* fChangelog = nullptr;
            _wfopen_s(&fChangelog, changelogPath.c_str(), L"wb");
            if (fChangelog) {
                std::string utf8Notes = WStringToUTF8(releaseNotes);
                fwrite(utf8Notes.c_str(), 1, utf8Notes.size(), fChangelog);
                fclose(fChangelog);
            }

            // Формируем командный скрипт
            std::wstring batPath = std::wstring(tempDirBuf) + L"rtx_updater.bat";
            FILE* fBat = nullptr;
            _wfopen_s(&fBat, batPath.c_str(), L"wb");
            if (fBat) {
                std::string batContent = "@echo off\r\n"
                    ":retry\r\n"
                    "timeout /t 1 /nobreak >nul\r\n"
                    "move /y \"" + WStringToUTF8(tempExePath) + "\" \"" + WStringToUTF8(currentExePath) + "\"\r\n"
                    "if errorlevel 1 goto retry\r\n"
                    "start \"\" \"" + WStringToUTF8(currentExePath) + "\" --updated\r\n"
                    "del \"%~f0\"\r\n";
                fwrite(batContent.c_str(), 1, batContent.size(), fBat);
                fclose(fBat);
            }

            SHELLEXECUTEINFOW sei = {};
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"open";
            sei.lpFile = batPath.c_str();
            sei.nShow = SW_HIDE;
            if (ShellExecuteExW(&sei)) {
                ExitProcess(0);
                return true;
            }
            else {
                logCb(L"[Updater] Не удалось запустить скрипт автозамены cmd.exe");
            }
        }
        catch (const std::exception& ex) {
            logCb(L"[Updater] Ошибка при обновлении: " + UTF8ToWString(ex.what()));
        }
        return false;
    }

private:
    static int ParseBuildNumber(const std::string& tag, const std::string& name) {
        std::string full = tag + " " + name;
        std::string lower = full;
        for (char &c : lower) c = (char)tolower((unsigned char)c);

        int maxBuildFound = 0;
        size_t pos = 0;

        while ((pos = lower.find_first_of("0123456789", pos)) != std::string::npos) {
            size_t start = pos;
            while (pos < lower.size() && isdigit((unsigned char)lower[pos])) {
                pos++;
            }
            int val = atoi(lower.substr(start, pos - start).c_str());

            bool isBuildToken = false;
            if (start >= 5 && lower.substr(start - 5, 5) == "build") isBuildToken = true;
            else if (start >= 6 && lower.substr(start - 6, 5) == "build") isBuildToken = true;
            else if (start >= 1 && (lower[start - 1] == 'b' || lower[start - 1] == '#' || lower[start - 1] == 'v')) isBuildToken = true;

            // Игнорируем обычные года в названии (например, 2026), если перед ними нет слова build
            if (val >= 2020 && val <= 2100 && !isBuildToken) {
                continue;
            }

            if (val > maxBuildFound) {
                maxBuildFound = val;
            }
        }
        return maxBuildFound;
    }

    static bool IsVersionNewer(const std::wstring& vNew, const std::wstring& vCurrent) {
        int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
        int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
        
        int matchNew = swscanf_s(vNew.c_str(), L"%d.%d.%d.%d", &n1, &n2, &n3, &n4);
        if (matchNew < 2) matchNew = swscanf_s(vNew.c_str(), L"%d.%d.%d", &n1, &n2, &n3);
        
        int matchCur = swscanf_s(vCurrent.c_str(), L"%d.%d.%d.%d", &c1, &c2, &c3, &c4);
        if (matchCur < 2) matchCur = swscanf_s(vCurrent.c_str(), L"%d.%d.%d", &c1, &c2, &c3);

        if (matchNew < 2 && matchCur < 2) {
            return vNew != vCurrent && vNew > vCurrent;
        }

        if (n1 != c1) return n1 > c1;
        if (n2 != c2) return n2 > c2;
        if (n3 != c3) return n3 > c3;
        if (n4 != c4) return n4 > c4;

        if (vNew != vCurrent) {
            bool curHasDash = (vCurrent.find(L"-") != std::wstring::npos);
            bool newHasDash = (vNew.find(L"-") != std::wstring::npos);
            
            if (curHasDash && !newHasDash) return true;
            if (curHasDash && newHasDash) return vNew > vCurrent;
            if (!curHasDash && newHasDash) return false;
            
            return vNew > vCurrent;
        }
        return false;
    }

    static std::wstring UTF8ToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
};
