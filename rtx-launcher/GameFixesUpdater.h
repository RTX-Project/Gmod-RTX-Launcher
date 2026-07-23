#pragma once
#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>
#include "ZipExtract.h" // Убедитесь, что этот заголовок уже существует и умеет распаковывать

namespace fs = std::filesystem;

class GameFixesUpdater {
public:
    using LogFn = std::function<void(const std::wstring&)>;
    using ProgressFn = std::function<void(float, const std::wstring&, const std::wstring&)>;

    static bool DownloadAndApplyFixes(const fs::path& destRoot, const std::wstring& repo, LogFn log, ProgressFn progress = nullptr) {
        log(L"Ищем свежий пакет фиксов (" + repo + L")...");

        HINTERNET hInternet = InternetOpenW(L"RTXLauncher/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if (!hInternet) {
            log(L"⚠ Ошибка: Не удалось инициализировать WinINet.");
            return false;
        }

        std::wstring url = L"https://api.github.com/repos/" + repo + L"/releases/latest";
        HINTERNET hConnect = InternetOpenUrlW(hInternet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hConnect) {
            log(L"⚠ Ошибка: Не удалось получить данные о релизах.");
            InternetCloseHandle(hInternet);
            return false;
        }

        std::string jsonStr;
        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            jsonStr.append(buffer, bytesRead);
        }
        InternetCloseHandle(hConnect);

        // Парсинг tag_name для проверки версии
        std::string tagName;
        size_t tagPos = jsonStr.find("\"tag_name\":");
        if (tagPos != std::string::npos) {
            size_t startQuote = jsonStr.find("\"", tagPos + 11);
            if (startQuote != std::string::npos) {
                size_t endQuote = jsonStr.find("\"", startQuote + 1);
                if (endQuote != std::string::npos) {
                    tagName = jsonStr.substr(startQuote + 1, endQuote - startQuote - 1);
                }
            }
        }

        fs::path versionFile = destRoot / L"fixes_version.txt";
        if (!tagName.empty() && fs::exists(versionFile)) {
            std::ifstream vf(versionFile);
            std::string localTag;
            if (std::getline(vf, localTag)) {
                if (localTag == tagName) {
                    log(L"✓ Установлена актуальная версия фиксов (" + std::wstring(tagName.begin(), tagName.end()) + L").");
                    InternetCloseHandle(hInternet);
                    return true;
                }
            }
        }

        // Простейший парсинг JSON для поиска URL архива (заканчивающегося на .zip, желательно с launcher в названии)
        std::string downloadUrl;
        size_t browserUrlPos = jsonStr.find("\"browser_download_url\":");
        while (browserUrlPos != std::string::npos) {
            size_t startQuote = jsonStr.find("\"", browserUrlPos + 23);
            if (startQuote != std::string::npos) {
                size_t endQuote = jsonStr.find("\"", startQuote + 1);
                if (endQuote != std::string::npos) {
                    std::string url = jsonStr.substr(startQuote + 1, endQuote - startQuote - 1);
                    if (url.find(".zip") != std::string::npos) {
                        downloadUrl = url;
                        // Если в названии есть 'launcher', то это приоритетный файл
                        if (url.find("launcher") != std::string::npos) {
                            break;
                        }
                    }
                }
            }
            browserUrlPos = jsonStr.find("\"browser_download_url\":", browserUrlPos + 23);
        }

        if (downloadUrl.empty()) {
            log(L"⚠ Не удалось найти ZIP-архив в релизе фиксов.");
            InternetCloseHandle(hInternet);
            return false;
        }

        std::wstring wDownloadUrl = std::wstring(downloadUrl.begin(), downloadUrl.end());
        log(L"Скачивание фиксов: " + wDownloadUrl);

        hConnect = InternetOpenUrlW(hInternet, wDownloadUrl.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hConnect) {
            log(L"⚠ Ошибка: Не удалось начать скачивание архива фиксов.");
            InternetCloseHandle(hInternet);
            return false;
        }

        fs::path tempZipPath = destRoot / L"fixes_temp.zip";
        std::ofstream zipFile(tempZipPath, std::ios::binary);
        if (!zipFile) {
            log(L"⚠ Ошибка: Не удалось создать временный файл для скачивания.");
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }

        DWORD contentLength = 0;
        DWORD lengthSize = sizeof(contentLength);
        HttpQueryInfoW(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &lengthSize, nullptr);

        size_t totalBytes = 0;
        ULONGLONG startTick = GetTickCount64();
        ULONGLONG lastUpdateTick = startTick;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            zipFile.write(buffer, bytesRead);
            totalBytes += bytesRead;

            ULONGLONG currentTick = GetTickCount64();
            if (progress && currentTick - lastUpdateTick >= 100) {
                lastUpdateTick = currentTick;
                float p = contentLength > 0 ? (float)totalBytes / contentLength : -1.0f;
                
                double elapsedSeconds = (currentTick - startTick) / 1000.0;
                double speed = elapsedSeconds > 0 ? (totalBytes / 1024.0 / 1024.0) / elapsedSeconds : 0.0;
                
                std::wstring stats;
                if (contentLength > 0 && speed > 0.0) {
                    double remainingMbytes = (contentLength - totalBytes) / 1024.0 / 1024.0;
                    double etaSeconds = remainingMbytes / speed;
                    
                    wchar_t buf[256];
                    swprintf(buf, 256, L"Скачано: %.1f / %.1f МБ | Скорость: %.1f МБ/с | Осталось: %.0f сек", 
                        totalBytes / 1024.0 / 1024.0, contentLength / 1024.0 / 1024.0, speed, etaSeconds);
                    stats = buf;
                } else {
                    wchar_t buf[256];
                    swprintf(buf, 256, L"Скачано: %.1f МБ", totalBytes / 1024.0 / 1024.0);
                    stats = buf;
                }
                progress(p, L"Скачивание фиксов...", stats);
            }
        }
        
        if (progress) {
            progress(1.0f, L"Распаковка архива...", L"");
        }
        zipFile.close();
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        log(L"✓ Скачано " + std::to_wstring(totalBytes / 1024) + L" КБ. Начинается распаковка...");

        bool extracted = false;
        try {
            ZipExtract::extractAndFlatten(tempZipPath.wstring(), destRoot.wstring());
            extracted = true;
            if (!tagName.empty()) {
                std::ofstream vf(versionFile, std::ios::trunc);
                vf << tagName;
            }
        } catch (const std::exception& e) {
            log(L"⚠ Ошибка при распаковке: " + std::wstring(e.what(), e.what() + strlen(e.what())));
        }
        
        // Удаляем временный файл
        std::error_code ec;
        fs::remove(tempZipPath, ec);

        if (extracted) {
            log(L"✓ Пакет фиксов успешно установлен!");
            return true;
        } else {
            log(L"⚠ Ошибка при распаковке пакета фиксов.");
            return false;
        }
    }
};
