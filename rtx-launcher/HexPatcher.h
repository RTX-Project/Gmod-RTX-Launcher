#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <functional>

namespace fs = std::filesystem;

class HexPatcher {
private:
    static std::vector<uint8_t> HexToBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            if (hex[i] == '?') {
                bytes.push_back(0); // Wildcard
                continue;
            }
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    static std::vector<bool> HexToMask(const std::string& hex) {
        std::vector<bool> mask;
        for (size_t i = 0; i < hex.length(); i += 2) {
            if (hex[i] == '?') {
                mask.push_back(false);
            } else {
                mask.push_back(true);
            }
        }
        return mask;
    }

    static int FindPattern(const std::vector<uint8_t>& data, const std::string& hexPattern, int start = 0) {
        std::vector<uint8_t> pattern = HexToBytes(hexPattern);
        std::vector<bool> mask = HexToMask(hexPattern);
        
        for (size_t i = start; i <= data.size() - pattern.size(); ++i) {
            bool found = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                if (mask[j] && data[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return (int)i;
        }
        return -1;
    }

public:
    struct Patch {
        std::string searchPattern;
        int offset;
        std::string replacementHex;
    };

    static bool ApplyPatches(const fs::path& destRoot, std::function<void(const std::wstring&)> log) {
        log(L"Применение бинарных патчей SourceRTXTweaks для 64-битной версии...");

        struct FilePatchGroup {
            std::wstring filename;
            std::vector<Patch> patches;
        };

        std::vector<FilePatchGroup> patches64 = {
            { L"bin\\win64\\engine.dll", {
                { "4883ec480f10", 0, "31c0c3" },
                { "753cf30f10", 0, "eb" },
                { "7e5244", 0, "eb" },
                { "753c498b4204", 0, "eb" }
            }},
            { L"bin\\win64\\shaderapidx9.dll", {
                { "480f4ec1c7", 0, "90909090" },
                { "4833cce8????03004881c448", 0, "85c0750466b80400" },
                { "4883ec084c", 0, "31c0c3" }
            }},
            { L"garrysmod\\bin\\win64\\client.dll", {
                { "4883ec480f1022", 0, "31c0c3" },
                { "0fb68154", 0, "b001c3" }
            }},
            { L"bin\\win64\\materialsystem.dll", {
                { "f77c24683bc10f4fc1488b8c24300100004833cce8????04004881c448010000", 0, 
                  "448b4424684585c0740341f7f839c80f4fc14881c448010000c3" }
            }},
            { L"bin\\win64\\datacache.dll", {
                { "647838302e767478", 0, "647839302e767478" }
            }}
        };

        bool allSuccess = true;

        for (const auto& group : patches64) {
            fs::path filePath = destRoot / group.filename;
            
            if (!fs::exists(filePath)) {
                // Пытаемся найти client.dll в корневом bin, если его нет в garrysmod/bin
                if (group.filename == L"garrysmod\\bin\\win64\\client.dll") {
                    filePath = destRoot / L"bin\\win64\\client.dll";
                    if (!fs::exists(filePath)) {
                        log(L"⚠ Не найден файл: " + group.filename);
                        allSuccess = false;
                        continue;
                    }
                } else {
                    log(L"⚠ Не найден файл: " + group.filename);
                    allSuccess = false;
                    continue;
                }
            }

            std::ifstream inFile(filePath, std::ios::binary);
            if (!inFile) {
                log(L"⚠ Ошибка чтения: " + group.filename);
                allSuccess = false;
                continue;
            }
            
            std::vector<uint8_t> data((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            inFile.close();

            bool fileModified = false;

            for (const auto& patch : group.patches) {
                int pos = 0;
                while (true) {
                    pos = FindPattern(data, patch.searchPattern, pos);
                    if (pos == -1) break;
                    
                    std::vector<uint8_t> replBytes = HexToBytes(patch.replacementHex);
                    for (size_t i = 0; i < replBytes.size(); i++) {
                        data[pos + patch.offset + i] = replBytes[i];
                    }
                    fileModified = true;
                    pos += static_cast<int>(patch.searchPattern.length() / 2);
                }
            }

            if (fileModified) {
                // Создаём бекап, если его ещё нет
                fs::path backupPath = filePath;
                backupPath += L".rtx_backup";
                if (!fs::exists(backupPath)) {
                    fs::copy_file(filePath, backupPath);
                }

                std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
                if (outFile) {
                    outFile.write((const char*)data.data(), data.size());
                    outFile.close();
                    log(L"✓ Пропатчен файл: " + group.filename);
                } else {
                    log(L"⚠ Ошибка записи: " + group.filename);
                    allSuccess = false;
                }
            }
        }
        
        return allSuccess;
    }
};
