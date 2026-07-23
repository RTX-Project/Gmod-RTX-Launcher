// RtxRemixUpdater.h - проверка последнего релиза RTX Remix на GitHub и
// применение обновления: скачивание архива целиком (ограничение GitHub -
// частичной докачки одного файла релиза не бывает), но копирование в
// рабочую папку ТОЛЬКО файлов, реально отличающихся по SHA-1 от уже
// имеющихся.
#pragma once
#include <windows.h>
#include <string>
#include <filesystem>
#include <functional>
#include <fstream>
#include "HttpClient.h"
#include "JsonValue.h"
#include "ZipExtract.h"
#include "Sha1.h"

namespace fs = std::filesystem;

struct RtxReleaseInfo {
    std::wstring tagName;
    std::wstring assetName;
    std::wstring assetUrl;
    uint64_t assetSize = 0;
    std::wstring htmlUrl;
    std::wstring publishedAt;
    std::wstring body;
    bool valid = false;
};

struct RtxUpdateCheckResult {
    std::wstring currentTag;
    std::wstring latestTag;
    bool hasUpdate = false;
    bool ok = false;
    RtxReleaseInfo release;
};

class RtxRemixUpdater {
public:
    std::wstring owner = L"NVIDIAGameWorks"; // ПРОВЕРЬТЕ и подставьте нужный репозиторий сборки RTX Remix
    std::wstring repo = L"rtx-remix";
    std::wstring userAgent = L"MetrostroiRTXLauncher/1.0";

    using LogFn = std::function<void(const std::wstring&)>;
    using ProgressFn = std::function<bool(uint64_t, uint64_t)>;

    RtxUpdateCheckResult checkForUpdate(const fs::path& launcherRoot, LogFn log) {
        RtxUpdateCheckResult result;
        result.currentTag = loadCurrentTag(launcherRoot);

        std::wstring url = L"https://api.github.com/repos/" + owner + L"/" + repo + L"/releases/latest";
        log(L"Проверяю последнюю версию RTX Remix на GitHub...");
        std::string body;
        try {
            body = HttpClient::getText(url, userAgent);
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] ") + toWide(e.what()));
            return result;
        }

        JsonValue json;
        try {
            json = JsonValue::parse(body);
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] Не удалось разобрать ответ GitHub: ") + toWide(e.what()));
            return result;
        }

        result.release.tagName = toWide(json[std::string("tag_name")].asString());
        if (result.release.tagName.empty()) {
            log(L"[ОШИБКА] В ответе GitHub не найден tag_name (возможно, релизов ещё нет "
                L"или указан неверный репозиторий в настройках owner/repo).");
            return result;
        }

        const JsonValue& assets = json[std::string("assets")];
        if (assets.isArray()) {
            for (auto& a : assets.arrayValue) {
                std::string name = a[std::string("name")].asString();
                if (name.size() >= 4 && name.substr(name.size() - 4) == ".zip") {
                    result.release.assetName = toWide(name);
                    result.release.assetUrl = toWide(a[std::string("browser_download_url")].asString());
                    result.release.assetSize = (uint64_t)a[std::string("size")].asInt64();
                    result.release.valid = true;
                    break;
                }
            }
            if (!result.release.valid && !assets.arrayValue.empty()) {
                auto& a = assets.arrayValue.front();
                result.release.assetName = toWide(a[std::string("name")].asString());
                result.release.assetUrl = toWide(a[std::string("browser_download_url")].asString());
                result.release.assetSize = (uint64_t)a[std::string("size")].asInt64();
                result.release.valid = true;
            }
        }

        result.latestTag = result.release.tagName;
        result.hasUpdate = (!result.latestTag.empty() && result.latestTag != result.currentTag);
        result.ok = true;

        log(L"Текущая версия (записанная у нас): " + (result.currentTag.empty() ? L"(нет данных)" : result.currentTag));
        log(L"Последняя версия на GitHub:         " + result.latestTag);
        log(result.hasUpdate ? L">>> Доступно обновление RTX Remix." : L"Обновлений нет — скачивание не требуется.");
        return result;
    }

    // Получает список последних `count` релизов с GitHub API.
    // Использует существующий HttpClient и возвращает вектор структур RtxReleaseInfo.
    std::vector<RtxReleaseInfo> listReleases(int count, LogFn log) {
        std::vector<RtxReleaseInfo> releases;
        
        // Формируем URL с параметром per_page, чтобы не скачивать больше, чем нужно
        std::wstring url = L"https://api.github.com/repos/" + owner + L"/" + repo + L"/releases?per_page=" + std::to_wstring(count);
        
        std::string bodyText;
        try {
            // Выполняем GET запрос, не забыв передать User-Agent (обязательно для GitHub API)
            bodyText = HttpClient::getText(url, userAgent);
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] Не удалось получить список релизов: ") + toWide(e.what()));
            return releases;
        }

        JsonValue json;
        try {
            // Разбираем JSON-ответ. JsonValue умеет парсить корневые массивы.
            json = JsonValue::parse(bodyText);
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] Не удалось разобрать ответ GitHub: ") + toWide(e.what()));
            return releases;
        }

        if (json.isArray()) {
            for (auto& item : json.arrayValue) {
                RtxReleaseInfo info;
                std::string tagName = item[std::string("tag_name")].asString();
                std::string name = item[std::string("name")].asString();
                
                // Если tag_name пустой, используем поле name как запасной вариант
                info.tagName = toWide(tagName.empty() ? name : tagName);
                info.htmlUrl = toWide(item[std::string("html_url")].asString());
                
                // Обрезаем дату публикации (published_at) до YYYY-MM-DD
                std::string pubAt = item[std::string("published_at")].asString();
                if (pubAt.length() >= 10) pubAt = pubAt.substr(0, 10);
                info.publishedAt = toWide(pubAt);
                
                info.body = toWide(item[std::string("body")].asString());
                
                // Ищем подходящий ассет для скачивания (нам нужен .zip файл)
                const JsonValue& assets = item[std::string("assets")];
                if (assets.isArray()) {
                    for (auto& a : assets.arrayValue) {
                        std::string assetName = a[std::string("name")].asString();
                        if (assetName.size() >= 4 && assetName.substr(assetName.size() - 4) == ".zip") {
                            info.assetName = toWide(assetName);
                            info.assetUrl = toWide(a[std::string("browser_download_url")].asString());
                            info.assetSize = (uint64_t)a[std::string("size")].asInt64();
                            info.valid = true;
                            break;
                        }
                    }
                    if (!info.valid && !assets.arrayValue.empty()) {
                        auto& a = assets.arrayValue.front();
                        info.assetName = toWide(a[std::string("name")].asString());
                        info.assetUrl = toWide(a[std::string("browser_download_url")].asString());
                        info.assetSize = (uint64_t)a[std::string("size")].asInt64();
                        info.valid = true;
                    }
                }
                
                releases.push_back(info);
                if (releases.size() >= count) break;
            }
        }

        return releases;
    }

    std::wstring loadCurrentTag(const fs::path& launcherRoot) const {
        std::wifstream f(versionFile(launcherRoot));
        if (!f) return L"";
        std::wstring tag;
        std::getline(f, tag);
        return tag;
    }

    struct ApplyStats { int applied = 0; int skipped = 0; int errors = 0; };

    ApplyStats downloadAndApply(const fs::path& destRoot, const fs::path& launcherRoot, const RtxReleaseInfo& release,
                                 LogFn log, ProgressFn onProgress) {
        ApplyStats stats;
        if (!release.valid) {
            log(L"[ОШИБКА] В релизе не найдено файлов для загрузки.");
            stats.errors = 1;
            return stats;
        }

        wchar_t tempDir[MAX_PATH];
        GetTempPathW(MAX_PATH, tempDir);
        fs::path work = fs::path(tempDir) / L"MetrostroiRtxUpdate";
        std::error_code ec;
        fs::remove_all(work, ec);
        fs::create_directories(work, ec);

        fs::path zipPath = work / release.assetName;
        log(L"Скачиваю: " + release.assetName + L" ...");
        try {
            HttpClient::downloadFile(release.assetUrl, zipPath.wstring(), userAgent, onProgress);
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] Загрузка не удалась: ") + toWide(e.what()));
            stats.errors++;
            return stats;
        }
        log(L"Загрузка завершена.");

        fs::path extractDir = work / L"extracted";
        log(L"Распаковываю архив...");
        try {
            ZipExtract::extractAll(zipPath.wstring(), extractDir.wstring());
        } catch (const std::exception& e) {
            log(std::wstring(L"[ОШИБКА] Распаковка не удалась: ") + toWide(e.what()));
            stats.errors++;
            return stats;
        }

        log(L"Сравниваю файлы и применяю только изменения...");
        fs::path destRtx = destRoot / L"rtx-remix";
        for (auto it = fs::recursive_directory_iterator(extractDir, fs::directory_options::skip_permission_denied, ec);
             it != fs::recursive_directory_iterator(); it.increment(ec)) {
            if (ec) break;
            if (!it->is_regular_file(ec)) continue;
            fs::path rel = fs::relative(it->path(), extractDir, ec);
            fs::path dst = destRtx / rel;

            bool needCopy = true;
            if (fs::exists(dst, ec)) {
                bool ok1 = false, ok2 = false;
                std::string h1 = Sha1::hashFile(it->path().wstring(), &ok1);
                std::string h2 = Sha1::hashFile(dst.wstring(), &ok2);
                if (ok1 && ok2 && h1 == h2) needCopy = false;
            }

            if (needCopy) {
                fs::create_directories(dst.parent_path(), ec);
                fs::copy_file(it->path(), dst, fs::copy_options::overwrite_existing, ec);
                if (ec) { stats.errors++; log(L"[ОШИБКА] " + rel.wstring()); }
                else { stats.applied++; log(L"[ОБНОВЛЕНО] rtx-remix\\" + rel.wstring()); }
            } else {
                stats.skipped++;
            }
        }

        saveCurrentTag(launcherRoot, release.tagName);
        log(L"Готово. Применено файлов: " + std::to_wstring(stats.applied) +
            L", без изменений: " + std::to_wstring(stats.skipped) +
            L", ошибок: " + std::to_wstring(stats.errors));

        fs::remove_all(work, ec);
        return stats;
    }

private:
    static std::wstring toWide(const std::string& s) {
        if (s.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring out(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], size);
        return out;
    }

    fs::path versionFile(const fs::path& launcherRoot) const {
        return launcherRoot / L".rtx_remix_version.txt";
    }

    void saveCurrentTag(const fs::path& launcherRoot, const std::wstring& tag) const {
        std::wofstream f(versionFile(launcherRoot), std::ios::trunc);
        f << tag;
    }
};
