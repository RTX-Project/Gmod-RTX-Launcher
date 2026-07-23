#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>

namespace fs = std::filesystem;

class BinaryPatcher {
public:
    using LogFn = std::function<void(const std::wstring&)>;

    struct PatchDef {
        std::string pattern;
        int offset;
        std::string replacement;
    };

    struct FilePatch {
        std::wstring filename;
        std::vector<PatchDef> patches;
    };

    static bool ApplyRtxTweaks(const fs::path& destRoot, LogFn log) {
        // Патчи из SourceRTXTweaks для Garry's Mod (win64)
        std::vector<FilePatch> filesToPatch = {
            { L"bin/win64/engine.dll", {
                { "4883ec480f10", 0, "31c0c3" }, // c_frustumcull patches
                { "753cf30f10", 0, "eb" }, // brush entity backfaces
                { "7e5244", 0, "eb" }, // world backfaces
                { "753c498b4204", 0, "eb" }, // world backfaces
            } },
            { L"bin/win64/shaderapidx9.dll", {
                { "480f4ec1c7", 0, "90909090" }, // four hardware lights
                { "4833cce8????03004881c448", 0, "85c0750466b80400" }, // zero sized buffer
                { "4883ec084c", 0, "31c0c3" } // shader constants
            } },
            { L"bin/win64/client.dll", {
                { "4883ec480f1022", 0, "31c0c3" }, // c_frustumcull
                { "0fb68154", 0, "b001c3" }, // r_forcenovis [getter]
            } },
            { L"bin/win64/materialsystem.dll", {
                { "f77c24683bc10f4fc1488b8c24300100004833cce8????04004881c448010000", 0, "448b4424684585c0740341f7f839c80f4fc14881c448010000c3" }, // zero sized buffer protection
            } },
            { L"bin/win64/datacache.dll", {
                { "647838302e767478", 0, "647839302e767478" }, // force load dx9 vtx
            } }
        };

        bool allSuccess = true;
        for (const auto& filePatch : filesToPatch) {
            fs::path filePath = destRoot / filePatch.filename;
            if (!fs::exists(filePath)) {
                log(L"⚠ Файл для патчинга не найден: " + filePatch.filename);
                continue; // Файла может не быть, это нормально
            }

            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if (!file) {
                log(L"⚠ Не удалось открыть файл для чтения: " + filePatch.filename);
                allSuccess = false;
                continue;
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(size);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                log(L"⚠ Не удалось прочитать файл: " + filePatch.filename);
                allSuccess = false;
                continue;
            }
            file.close();

            bool fileModified = false;
            for (size_t i = 0; i < filePatch.patches.size(); ++i) {
                const auto& patch = filePatch.patches[i];
                size_t offset = FindPattern(buffer, patch.pattern);
                if (offset != std::string::npos) {
                    // Проверяем, есть ли дубликаты
                    size_t offset2 = FindPattern(buffer, patch.pattern, offset + 1);
                    if (offset2 != std::string::npos) {
                        log(L"⚠ Патч " + std::to_wstring(i + 1) + L" в " + filePatch.filename + L" найден несколько раз! Пропуск.");
                        allSuccess = false;
                        continue;
                    }

                    // Применяем патч
                    std::vector<uint8_t> replacementBytes = HexToBytes(patch.replacement);
                    size_t targetPos = offset + patch.offset;
                    if (targetPos + replacementBytes.size() <= buffer.size()) {
                        std::copy(replacementBytes.begin(), replacementBytes.end(), buffer.begin() + targetPos);
                        fileModified = true;
                        log(L"✓ Успешно применен патч " + std::to_wstring(i + 1) + L" к " + filePatch.filename);
                    } else {
                        log(L"⚠ Ошибка: выход за пределы файла при патчинге " + filePatch.filename);
                        allSuccess = false;
                    }
                } else {
                    // Это может быть нормально, если файл уже пропатчен
                    log(L"⚠ Паттерн " + std::to_wstring(i + 1) + L" не найден в " + filePatch.filename + L" (возможно, уже пропатчен)");
                }
            }

            if (fileModified) {
                std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
                if (!outFile) {
                    log(L"⚠ Не удалось открыть файл для записи: " + filePatch.filename);
                    allSuccess = false;
                    continue;
                }
                if (!outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size())) {
                    log(L"⚠ Ошибка записи файла: " + filePatch.filename);
                    allSuccess = false;
                }
            }
        }
        return allSuccess;
    }

private:
    static std::vector<uint8_t> HexToBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    static size_t FindPattern(const std::vector<uint8_t>& data, const std::string& patternStr, size_t start = 0) {
        std::vector<int> pattern;
        for (size_t i = 0; i < patternStr.length(); i += 2) {
            std::string byteString = patternStr.substr(i, 2);
            if (byteString == "??") {
                pattern.push_back(-1); // Wildcard
            } else {
                pattern.push_back(strtol(byteString.c_str(), nullptr, 16));
            }
        }

        if (pattern.empty() || data.size() < pattern.size()) return std::string::npos;

        for (size_t i = start; i <= data.size() - pattern.size(); ++i) {
            bool found = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                if (pattern[j] != -1 && data[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
        return std::string::npos;
    }
};
