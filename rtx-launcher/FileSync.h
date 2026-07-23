// FileSync.h - копирование файлов игры из Ð¸ÑÑ…Ð¾Ð´Ð½Ð¾Ð¹ ÑƒÑÑ‚Ð°Ð½Ð¾Ð²ÐºÐ¸ в рабочую
// (модифицируемую) папку. При повторном Ð·Ð°Ð¿ÑƒÑÐºÐµ ÐºÐ¾Ð¿Ð¸Ñ€ÑƒÑŽÑ‚ÑÑ только
// Ð½Ð¾Ð²Ñ‹Ðµ/Ð¸Ð·Ð¼ÐµÐ½Ð¸Ð²ÑˆÐ¸ÐµÑÑ файлы (ÑÑ€Ð°Ð²Ð½ÐµÐ½Ð¸Ðµ размера+времени изменения, либо
// по SHA-1, если включена ÑÐ¾Ð¾Ñ‚Ð²ÐµÑ‚ÑÑ‚Ð²ÑƒÑŽÑ‰Ð°Ñ Ð¾Ð¿Ñ†Ð¸Ñ).
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <algorithm>
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <string>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>
#include "Sha1.h"

struct SpeedSample {
    std::chrono::steady_clock::time_point time;
    uint64_t bytes;
};

namespace fs = std::filesystem;

struct SyncStats {
    int copied = 0;
    int updated = 0;
    int skipped = 0;
    int deleted = 0;
    int errors = 0;
};

class FileSync {
public:
    // Папки верхнего ÑƒÑ€Ð¾Ð²Ð½Ñ и отдельные файлы, которые нужно ÑÐ¸Ð½Ñ…Ñ€Ð¾Ð½Ð¸Ð·Ð¸Ñ€Ð¾Ð²Ð°Ñ‚ÑŒ
    // (согласно ÑÑ‚Ñ€ÑƒÐºÑ‚ÑƒÑ€Ðµ ÑƒÑÑ‚Ð°Ð½Ð¾Ð²ÐºÐ¸ Garry's Mod / RTX Remix).
    std::vector<std::wstring> foldersToSync = { L"bin", L"garrysmod", L"platform", L"sourceengine" };
    std::vector<std::wstring> filesToSync = { L"dxvk.conf", L"gmod.exe", L"hl2.exe", L"installed_packages.json",
                                               L"rtx.conf", L"steam_appid.txt", L"user.conf" };

    // ÐžÑ‚Ð½Ð¾ÑÐ¸Ñ‚ÐµÐ»ÑŒÐ½Ñ‹Ðµ под-пути (от ÐºÐ¾Ñ€Ð½Ñ Ð¸ÑÑ‚Ð¾Ñ‡Ð½Ð¸ÐºÐ°), которые Ð¸ÑÐºÐ»ÑŽÑ‡Ð°ÑŽÑ‚ÑÑ из
    // ÑÐ¸Ð½Ñ…Ñ€Ð¾Ð½Ð¸Ð·Ð°Ñ†Ð¸Ð¸ (ÐºÑÑˆ, загрузки, ÑÐºÑ€Ð¸Ð½ÑˆÐ¾Ñ‚Ñ‹ и т.п. - раздувают копию и не
    // нужны для модификации).
    std::vector<std::wstring> excludeRelativePrefixes = {
        L"garrysmod\\cache", L"garrysmod\\download", L"garrysmod\\downloadlists",
        L"garrysmod\\screenshots", L"garrysmod\\demos", L"garrysmod\\dupes",
        L"rtx-remix" // Защищаем папку rtx-remix от Ð¿ÐµÑ€ÐµÐ·Ð°Ð¿Ð¸ÑÐ¸/ÑƒÐ´Ð°Ð»ÐµÐ½Ð¸Ñ
    };

    bool verifyHash = false;
    bool deleteRemoved = true;

    using LogFn = std::function<void(const std::wstring&)>;
    using ShouldStopFn = std::function<bool()>;

    // Ð’Ñ‹Ð¿Ð¾Ð»Ð½ÑÐµÑ‚ ÑÐ¸Ð½Ñ…Ñ€Ð¾Ð½Ð¸Ð·Ð°Ñ†Ð¸ÑŽ файлов из sourceRoot в destRoot.
    // Сначала Ð·Ð°Ð³Ñ€ÑƒÐ¶Ð°ÐµÑ‚ÑÑ Ð¼Ð°Ð½Ð¸Ñ„ÐµÑÑ‚ (ÑÐ¾ÑÑ‚Ð¾ÑÐ½Ð¸Ðµ файлов при прошлом Ð·Ð°Ð¿ÑƒÑÐºÐµ), 
    // затем Ð¾Ð±Ñ…Ð¾Ð´ÑÑ‚ÑÑ все разрешенные файлы Ð¸ÑÑ‚Ð¾Ñ‡Ð½Ð¸ÐºÐ°. ÐšÐ¾Ð¿Ð¸Ñ€ÑƒÑŽÑ‚ÑÑ только 
    // Ð¸Ð·Ð¼ÐµÐ½Ð¸Ð²ÑˆÐ¸ÐµÑÑ или новые файлы. Ð•ÑÐ»Ð¸ включено удаление (deleteRemoved), 
    // файлы, Ð¸ÑÑ‡ÐµÐ·Ð½ÑƒÐ²ÑˆÐ¸Ðµ из Ð¸ÑÑ‚Ð¾Ñ‡Ð½Ð¸ÐºÐ°, ÑƒÐ´Ð°Ð»ÑÑŽÑ‚ÑÑ из рабочей папки.
    SyncStats sync(const fs::path& sourceRoot, const fs::path& destRoot, const fs::path& launcherRoot,
                    LogFn log, ShouldStopFn shouldStop, std::function<void(float, const std::wstring&)> progressFn = nullptr, std::function<void()> pauseCheckFn = nullptr) {
        SyncStats stats;
        fs::create_directories(destRoot);

        int totalFiles = 0;
        uint64_t totalBytes = 0;
        forEachSourceFile(sourceRoot, [&](const fs::path& relPath) {
            totalFiles++;
            std::error_code ec;
            totalBytes += fs::file_size(sourceRoot / relPath, ec);
            return true;
        });

        auto startTime = std::chrono::steady_clock::now();
        double pausedSeconds = 0.0;
        uint64_t processedBytes = 0;

        // Загружаем предыдущее ÑÐ¾ÑÑ‚Ð¾ÑÐ½Ð¸Ðµ, чтобы копировать только измененное
        auto manifest = loadManifest(launcherRoot);
        ManifestMap newManifest;
        std::set<std::wstring> seen;

        int processedFiles = 0;
        uint64_t lastProcessedBytes = 0;
        double currentSpeed = 0.0;
        auto lastUpdate = std::chrono::steady_clock::now();
        double smoothedEta = -1.0;
        std::deque<SpeedSample> speedSamples;
        forEachSourceFile(sourceRoot, [&](const fs::path& relPath) {
            if (shouldStop && shouldStop()) return false; // Ð¾ÑÑ‚Ð°Ð½Ð¾Ð²Ð¸Ñ‚ÑŒ обход
            
            if (pauseCheckFn) {
                auto pStart = std::chrono::steady_clock::now();
                pauseCheckFn();
                pausedSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - pStart).count();
            }
            if (shouldStop && shouldStop()) return false; // проверить снова после возможной паузы

            std::wstring key = relPath.wstring();
            seen.insert(key);

            fs::path srcPath = sourceRoot / relPath;
            fs::path dstPath = destRoot / relPath;

            std::error_code ec;
            uint64_t size = fs::file_size(srcPath, ec);
            if (ec) {
                log(L"[ÐžÐ¨Ð˜Ð‘ÐšÐ] Не ÑƒÐ´Ð°Ð»Ð¾ÑÑŒ прочитать размер: " + relPath.wstring());
                stats.errors++;
                return true;
            }
            auto ftime = fs::last_write_time(srcPath, ec);
            long long mtime = ec ? 0 : ftime.time_since_epoch().count();

            std::wostringstream fastKeyStream;
            fastKeyStream << size << L":" << mtime;
            std::wstring fastKey = fastKeyStream.str();

            auto it = manifest.find(key);
            bool needCopy = false;
            std::wstring reason;

            if (it == manifest.end()) {
                needCopy = true;
                reason = L"новый файл";
            } else if (!fs::exists(dstPath, ec)) {
                needCopy = true;
                reason = L"файл был удален локально";
            } else if (it->second.fastKey != fastKey) {
                if (verifyHash) {
                    std::error_code ec;
                    auto sz = fs::file_size(srcPath, ec);
                    auto mtime = fs::last_write_time(srcPath, ec).time_since_epoch().count();
                    std::string newHash = std::to_string(sz) + "_" + std::to_string(mtime);
                    if (newHash != it->second.hash) {
                        needCopy = true;
                        reason = L"Ð¸Ð·Ð¼ÐµÐ½Ð¸Ð»Ð¾ÑÑŒ ÑÐ¾Ð´ÐµÑ€Ð¶Ð¸Ð¼Ð¾Ðµ (хеш)";
                    }
                } else {
                    needCopy = true;
                    reason = L"Ð¸Ð·Ð¼ÐµÐ½Ð¸Ð»Ð¸ÑÑŒ размер/дата";
                }
            }

            ManifestEntry entry;
            entry.fastKey = fastKey;

            if (needCopy) {
                fs::create_directories(dstPath.parent_path(), ec);
                
                CopyContext ctx = {
                    processedBytes, totalBytes, processedFiles, totalFiles,
                    startTime, pausedSeconds, progressFn ? &progressFn : nullptr, 
                    shouldStop ? &shouldStop : nullptr, pauseCheckFn ? &pauseCheckFn : nullptr,
                    lastUpdate, lastProcessedBytes, currentSpeed, smoothedEta, &speedSamples
                };
                
                BOOL bRet = CopyFileExW(srcPath.wstring().c_str(), dstPath.wstring().c_str(), CopyProgressCallback, &ctx, nullptr, 0);
                lastUpdate = ctx.lastUiUpdate;
                lastProcessedBytes = ctx.lastProcessedBytes;
                currentSpeed = ctx.currentSpeed;
                smoothedEta = ctx.smoothedEta;
                if (!bRet) {
                    ec.assign(GetLastError(), std::system_category());
                    log(L"[ÐžÐ¨Ð˜Ð‘ÐšÐ] Копирование: " + relPath.wstring() + L" (" + ([](const std::string& s) { return std::wstring(s.begin(), s.end()); })(ec.message()) + L")");
                    stats.errors++;
                } else {
                    ec.clear();
                    pausedSeconds = ctx.pausedSeconds;
                    
                    if (it == manifest.end()) {
                        stats.copied++;
                        log(L"[КОПИЯ]     " + relPath.wstring() + L" - " + reason);
                    } else {
                        stats.updated++;
                        log(L"[ÐžÐ‘ÐÐžÐ’Ð›Ð•ÐÐž] " + relPath.wstring() + L" - " + reason);
                    }
                    if (verifyHash) {
                        std::error_code ec;
                        auto sz = fs::file_size(dstPath, ec);
                        auto mtime = fs::last_write_time(dstPath, ec).time_since_epoch().count();
                        entry.hash = std::to_string(sz) + "_" + std::to_string(mtime);
                    }
                }
            } else {
                stats.skipped++;
                if (verifyHash && it != manifest.end()) entry.hash = it->second.hash;
            }

            newManifest[key] = entry;
            
            processedBytes += size;
            processedFiles++;

            if (progressFn && totalBytes > 0) {
                auto now = std::chrono::steady_clock::now();
                auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
                
                if (elapsedMs >= 200 || processedFiles == totalFiles) {
                    lastUpdate = now;
                    speedSamples.push_back({ now, processedBytes });
                    while (speedSamples.size() > 1) {
                        double dtSec = std::chrono::duration_cast<std::chrono::milliseconds>(now - speedSamples.front().time).count() / 1000.0;
                        if (dtSec > 2.5) {
                            speedSamples.pop_front();
                        } else {
                            break;
                        }
                    }

                    double windowDt = std::chrono::duration_cast<std::chrono::milliseconds>(now - speedSamples.front().time).count() / 1000.0;
                    double speed = 0.0;
                    if (windowDt > 0.1) {
                        speed = ((processedBytes - speedSamples.front().bytes) / 1024.0 / 1024.0) / windowDt;
                    }

                    double totalMb = totalBytes / 1024.0 / 1024.0;
                    double downMb = processedBytes / 1024.0 / 1024.0;
                    
                    int eta = -1;
                    if (speed > 0.5) {
                        double targetEta = (totalMb - downMb) / speed;
                        if (smoothedEta < 0.0) {
                            smoothedEta = targetEta;
                        } else {
                            if (targetEta < smoothedEta) {
                                smoothedEta = smoothedEta * 0.85 + targetEta * 0.15;
                            } else {
                                smoothedEta = (std::min)(smoothedEta, smoothedEta * 0.999 + targetEta * 0.001);
                            }
                        }
                        eta = (int)(smoothedEta + 0.5);
                    } else if (smoothedEta >= 0.0) {
                        eta = (int)(smoothedEta + 0.5);
                    }
                    
                    wchar_t buf[256];
                    if (eta >= 0) {
                        if (eta <= 59) {
                            if (totalMb >= 1024.0) {
                                swprintf_s(buf, L"Файлы: %d/%d | %.2f/%.2f ГБ | %.1f МБ/с | Осталось около %dс", processedFiles, totalFiles, downMb / 1024.0, totalMb / 1024.0, speed, eta);
                            } else {
                                swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с | Осталось около %dс", processedFiles, totalFiles, downMb, totalMb, speed, eta);
                            }
                        } else {
                            if (totalMb >= 1024.0) {
                                swprintf_s(buf, L"Файлы: %d/%d | %.2f/%.2f ГБ | %.1f МБ/с | Осталось около %dм %dс", processedFiles, totalFiles, downMb / 1024.0, totalMb / 1024.0, speed, eta / 60, eta % 60);
                            } else {
                                swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с | Осталось около %dм %dс", processedFiles, totalFiles, downMb, totalMb, speed, eta / 60, eta % 60);
                            }
                        }
                    } else {
                        if (totalMb >= 1024.0) {
                            swprintf_s(buf, L"Файлы: %d/%d | %.2f/%.2f ГБ | %.1f МБ/с", processedFiles, totalFiles, downMb / 1024.0, totalMb / 1024.0, speed);
                        } else {
                            swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с", processedFiles, totalFiles, downMb, totalMb, speed);
                        }
                    }
                    progressFn((float)processedBytes / (float)totalBytes, buf);
                }
            }
            
            return true;
        });

        if (deleteRemoved && !(shouldStop && shouldStop())) {
            for (auto& kv : manifest) {
                if (seen.find(kv.first) == seen.end()) {
                    fs::path dstPath = destRoot / fs::path(kv.first);
                    std::error_code ec;
                    if (fs::exists(dstPath, ec) && fs::is_regular_file(dstPath, ec)) {
                        fs::remove(dstPath, ec);
                        if (!ec) {
                            stats.deleted++;
                            log(L"[Ð£Ð”ÐÐ›Ð•ÐÐž]   " + kv.first + L" (Ð¸ÑÑ‡ÐµÐ· в Ð¸ÑÑ‚Ð¾Ñ‡Ð½Ð¸ÐºÐµ)");
                        } else {
                            stats.errors++;
                            log(L"[ÐžÐ¨Ð˜Ð‘ÐšÐ] Удаление: " + kv.first);
                        }
                    }
                }
            }
        }

        saveManifest(launcherRoot, newManifest);
        if (progressFn && totalFiles > 0) {
            progressFn(1.0f, L"Завершение...");
        }

        return stats;
    }

private:
    struct CopyContext {
        uint64_t baseProcessedBytes;
        uint64_t totalBytes;
        int processedFiles;
        int totalFiles;
        std::chrono::steady_clock::time_point startTime;
        double pausedSeconds;
        const std::function<void(float, const std::wstring&)>* progressFn;
        const std::function<bool()>* shouldStop;
        const std::function<void()>* pauseCheckFn;
        std::chrono::steady_clock::time_point lastUiUpdate;
        uint64_t lastProcessedBytes;
        double currentSpeed;
        double smoothedEta{ -1.0 };
        std::deque<SpeedSample>* speedSamples{ nullptr };
    };

    static DWORD CALLBACK CopyProgressCallback(
        LARGE_INTEGER TotalFileSize,
        LARGE_INTEGER TotalBytesTransferred,
        LARGE_INTEGER StreamSize,
        LARGE_INTEGER StreamBytesTransferred,
        DWORD dwStreamNumber,
        DWORD dwCallbackReason,
        HANDLE hSourceFile,
        HANDLE hDestinationFile,
        LPVOID lpData
    ) {
        CopyContext* ctx = (CopyContext*)lpData;
        
        if (ctx->pauseCheckFn && *ctx->pauseCheckFn) {
            auto pStart = std::chrono::steady_clock::now();
            (*ctx->pauseCheckFn)();
            ctx->pausedSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - pStart).count();
        }

        if (ctx->shouldStop && (*ctx->shouldStop) && (*ctx->shouldStop)()) {
            return PROGRESS_CANCEL;
        }

        if (ctx->progressFn && (*ctx->progressFn) && ctx->totalBytes > 0) {
            auto now = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - ctx->lastUiUpdate).count();
            if (dt >= 200 || TotalBytesTransferred.QuadPart == TotalFileSize.QuadPart) {
                ctx->lastUiUpdate = now;
                uint64_t currentProcessedBytes = ctx->baseProcessedBytes + TotalBytesTransferred.QuadPart;
                
                if (ctx->speedSamples) {
                    ctx->speedSamples->push_back({ now, currentProcessedBytes });
                    while (ctx->speedSamples->size() > 1) {
                        double dtSec = std::chrono::duration_cast<std::chrono::milliseconds>(now - ctx->speedSamples->front().time).count() / 1000.0;
                        if (dtSec > 2.5) {
                            ctx->speedSamples->pop_front();
                        } else {
                            break;
                        }
                    }
                }

                double windowDt = (ctx->speedSamples && !ctx->speedSamples->empty()) ?
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - ctx->speedSamples->front().time).count() / 1000.0 : 0.0;
                double speed = 0.0;
                if (windowDt > 0.1 && ctx->speedSamples) {
                    speed = ((currentProcessedBytes - ctx->speedSamples->front().bytes) / 1024.0 / 1024.0) / windowDt;
                }
                ctx->lastProcessedBytes = currentProcessedBytes;
                
                double totalMb = ctx->totalBytes / 1024.0 / 1024.0;
                double downMb = currentProcessedBytes / 1024.0 / 1024.0;
                
                int eta = -1;
                if (speed > 0.5) {
                    double targetEta = (totalMb - downMb) / speed;
                    if (ctx->smoothedEta < 0.0) {
                        ctx->smoothedEta = targetEta;
                    } else {
                        if (targetEta < ctx->smoothedEta) {
                            ctx->smoothedEta = ctx->smoothedEta * 0.85 + targetEta * 0.15;
                        } else {
                            ctx->smoothedEta = (std::min)(ctx->smoothedEta, ctx->smoothedEta * 0.999 + targetEta * 0.001);
                        }
                    }
                    eta = (int)(ctx->smoothedEta + 0.5);
                } else if (ctx->smoothedEta >= 0.0) {
                    eta = (int)(ctx->smoothedEta + 0.5);
                }
                
                wchar_t buf[256];
                if (eta >= 0) {
                    if (eta <= 59) {
                        swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с | Осталось около %dс", ctx->processedFiles, ctx->totalFiles, downMb, totalMb, speed, eta);
                    } else {
                        swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с | Осталось около %dм %dс", ctx->processedFiles, ctx->totalFiles, downMb, totalMb, speed, eta / 60, eta % 60);
                    }
                } else {
                    swprintf_s(buf, L"Файлы: %d/%d | %.1f/%.1f МБ | %.1f МБ/с", ctx->processedFiles, ctx->totalFiles, downMb, totalMb, speed);
                }

                (*ctx->progressFn)((float)currentProcessedBytes / (float)ctx->totalBytes, buf);
            }
        }
        return PROGRESS_CONTINUE;
    }

    struct ManifestEntry {
        std::wstring fastKey;
        std::string hash;
    };
    using ManifestMap = std::map<std::wstring, ManifestEntry>;

    // (поиск по ключу теперь Ð½Ð°Ð¿Ñ€ÑÐ¼ÑƒÑŽ через ManifestMap::find, т.к. ÑÑ‚Ð¾ std::map)

    bool isExcluded(const fs::path& relPath) const {
        std::wstring norm = relPath.wstring();
        for (auto& c : norm) if (c == L'/') c = L'\\';
        for (auto& prefix : excludeRelativePrefixes) {
            if (norm == prefix || norm.rfind(prefix + L"\\", 0) == 0) return true;
        }
        std::wstring base = relPath.filename().wstring();
        if (base.rfind(L"rtx.conf.backup", 0) == 0) return true;
        return false;
    }

    void forEachSourceFile(const fs::path& sourceRoot, std::function<bool(const fs::path&)> cb) {
        for (auto& folder : foldersToSync) {
            fs::path absFolder = sourceRoot / folder;
            std::error_code ec;
            if (!fs::is_directory(absFolder, ec)) continue;
            for (auto it = fs::recursive_directory_iterator(absFolder, fs::directory_options::skip_permission_denied, ec);
                 it != fs::recursive_directory_iterator(); it.increment(ec)) {
                if (ec) break;
                if (!it->is_regular_file(ec)) continue;
                fs::path relPath = fs::relative(it->path(), sourceRoot, ec);
                if (ec) continue;
                if (isExcluded(relPath)) continue;
                if (!cb(relPath)) return;
            }
        }
        for (auto& fname : filesToSync) {
            fs::path absPath = sourceRoot / fname;
            std::error_code ec;
            if (fs::is_regular_file(absPath, ec)) {
                if (!cb(fs::path(fname))) return;
            }
        }
    }

    fs::path manifestPath(const fs::path& launcherRoot) const {
        return launcherRoot / L".launcher_manifest.tsv";
    }

    // Формат Ð¼Ð°Ð½Ð¸Ñ„ÐµÑÑ‚Ð°: Ð¿Ñ€Ð¾ÑÑ‚Ð¾Ð¹ TSV (relpath \t fastKey \t hash), по одной
    // записи на ÑÑ‚Ñ€Ð¾ÐºÑƒ - минимализм без Ð½ÐµÐ¾Ð±Ñ…Ð¾Ð´Ð¸Ð¼Ð¾ÑÑ‚Ð¸ JSON-библиотеки.
    ManifestMap loadManifest(const fs::path& launcherRoot) {
        ManifestMap result;
        std::wifstream f(manifestPath(launcherRoot));
        if (!f) return result;
        std::wstring line;
        while (std::getline(f, line)) {
            size_t t1 = line.find(L'\t');
            if (t1 == std::wstring::npos) continue;
            size_t t2 = line.find(L'\t', t1 + 1);
            std::wstring key = line.substr(0, t1);
            std::wstring fastKey = (t2 == std::wstring::npos) ? line.substr(t1 + 1) : line.substr(t1 + 1, t2 - t1 - 1);
            ManifestEntry e;
            e.fastKey = fastKey;
            if (t2 != std::wstring::npos) {
                std::wstring hashW = line.substr(t2 + 1);
                e.hash.clear();
                e.hash.reserve(hashW.size());
                for (wchar_t wc : hashW) e.hash.push_back(static_cast<char>(wc));
            }
            result[key] = e;
        }
        return result;
    }

    void saveManifest(const fs::path& launcherRoot, const ManifestMap& manifest) {
        std::wofstream f(manifestPath(launcherRoot), std::ios::trunc);
        for (auto& kv : manifest) {
            std::wstring hashW(kv.second.hash.begin(), kv.second.hash.end());
            f << kv.first << L"\t" << kv.second.fastKey << L"\t" << hashW << L"\n";
        }
    }

public:
    // Публичный алиас для Ð¸ÑÐ¿Ð¾Ð»ÑŒÐ·Ð¾Ð²Ð°Ð½Ð¸Ñ из main.cpp (поиск по ключу в manifest).
    using ManifestMapPublic = ManifestMap;
};
