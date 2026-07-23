#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <stdio.h>

extern "C" {
#include "lzma/C/7z.h"
#include "lzma/C/7zAlloc.h"
#include "lzma/C/7zCrc.h"
#include "lzma/C/7zFile.h"
}

class ArchiveExtract {
private:
    static WRes OutFile_OpenUtf16(CSzFile *p, const UInt16 *name) {
        return OutFile_OpenW(p, (LPCWSTR)name);
    }

    static WRes MyCreateDir(const UInt16 *name) {
        return CreateDirectoryW((LPCWSTR)name, NULL) ? 0 : GetLastError();
    }

public:
    static void extractAll(const std::wstring& archivePath, const std::wstring& destDir) {
        CreateDirectoryW(destDir.c_str(), nullptr);

        ISzAlloc allocImp = { SzAlloc, SzFree };
        ISzAlloc allocTempImp = { SzAlloc, SzFree };

        CFileInStream archiveStream;
        CLookToRead2 lookStream;
        CSzArEx db;
        SRes res;
        UInt16 *temp = NULL;
        size_t tempSize = 0;

        WRes wres = InFile_OpenW(&archiveStream.file, archivePath.c_str());
        if (wres != 0) {
            throw std::runtime_error("Cannot open 7z input file");
        }

        FileInStream_CreateVTable(&archiveStream);
        archiveStream.wres = 0;
        LookToRead2_CreateVTable(&lookStream, False);
        lookStream.buf = NULL;

        res = SZ_OK;
        const size_t kInputBufSize = ((size_t)1 << 18);

        lookStream.buf = (Byte *)ISzAlloc_Alloc(&allocImp, kInputBufSize);
        if (!lookStream.buf) {
            File_Close(&archiveStream.file);
            throw std::runtime_error("Memory allocation failed");
        }
        
        lookStream.bufSize = kInputBufSize;
        lookStream.realStream = &archiveStream.vt;
        LookToRead2_INIT(&lookStream)

        CrcGenerateTable();
        SzArEx_Init(&db);

        res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
        if (res != SZ_OK) {
            ISzAlloc_Free(&allocImp, lookStream.buf);
            File_Close(&archiveStream.file);
            throw std::runtime_error("SzArEx_Open failed");
        }

        UInt32 blockIndex = 0xFFFFFFFF;
        Byte *outBuffer = 0;
        size_t outBufferSize = 0;

        for (UInt32 i = 0; i < db.NumFiles; i++) {
            size_t offset = 0;
            size_t outSizeProcessed = 0;
            const BoolInt isDir = SzArEx_IsDir(&db, i);

            size_t len = SzArEx_GetFileNameUtf16(&db, i, NULL);
            if (len > tempSize) {
                SzFree(NULL, temp);
                tempSize = len;
                temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
                if (!temp) {
                    res = SZ_ERROR_MEM;
                    break;
                }
            }

            SzArEx_GetFileNameUtf16(&db, i, temp);

            if (isDir) continue; // We create directories when writing files

            res = SzArEx_Extract(&db, &lookStream.vt, i,
                &blockIndex, &outBuffer, &outBufferSize,
                &offset, &outSizeProcessed,
                &allocImp, &allocTempImp);
            
            if (res != SZ_OK) break;

            CSzFile outFile;
            size_t processedSize = outSizeProcessed;
            
            std::wstring destFilePath = destDir + L"\\" + (wchar_t*)temp;
            // Replace / with \ in path
            for (auto& c : destFilePath) {
                if (c == L'/') c = L'\\';
            }

            // Create parent directories
            std::filesystem::path targetPath(destFilePath);
            std::filesystem::create_directories(targetPath.parent_path());

            wres = OutFile_OpenUtf16(&outFile, (const UInt16*)destFilePath.c_str());
            if (wres == 0) {
                File_Write(&outFile, outBuffer + offset, &processedSize);
                File_Close(&outFile);
            }
        }

        ISzAlloc_Free(&allocImp, outBuffer);
        SzFree(NULL, temp);
        SzArEx_Free(&db, &allocImp);
        ISzAlloc_Free(&allocImp, lookStream.buf);
        File_Close(&archiveStream.file);

        if (res != SZ_OK) {
            throw std::runtime_error("Error during 7z extraction");
        }
    }

    static void extractAndFlatten(const std::wstring& archivePath, const std::wstring& destDir) {
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        
        std::wstring extractTempDir = std::wstring(tempPath) + L"rtx_launcher_extract_temp_" + std::to_wstring(GetCurrentProcessId()) + L"_" + std::to_wstring(GetTickCount64());
        CreateDirectoryW(extractTempDir.c_str(), nullptr);
        
        // Extract to temporary folder
        extractAll(archivePath, extractTempDir);
        
        // Find root folder
        std::filesystem::path trueRoot = extractTempDir;
        std::vector<std::filesystem::path> children;
        for (const auto& entry : std::filesystem::directory_iterator(extractTempDir)) {
            children.push_back(entry.path());
        }
        if (children.size() == 1 && std::filesystem::is_directory(children[0])) {
            trueRoot = children[0];
        }
        
        // Move all files to destination
        std::filesystem::create_directories(destDir);
        for (const auto& entry : std::filesystem::recursive_directory_iterator(trueRoot)) {
            if (entry.is_regular_file()) {
                auto relPath = std::filesystem::relative(entry.path(), trueRoot);
                auto targetPath = std::filesystem::path(destDir) / relPath;
                std::filesystem::create_directories(targetPath.parent_path());
                
                std::error_code ec;
                if (std::filesystem::exists(targetPath)) {
                    std::wstring relStr = relPath.wstring();
                    for (auto& c : relStr) c = towlower(c);
                    if (relStr.find(L"rtx-remix") != std::wstring::npos) {
                        continue; // Do not touch rtx-remix folder
                    }
                    std::filesystem::remove(targetPath, ec);
                }
                std::filesystem::rename(entry.path(), targetPath, ec);
                if (ec) {
                    std::filesystem::copy_file(entry.path(), targetPath, std::filesystem::copy_options::overwrite_existing, ec);
                }
            }
        }
        
        std::error_code ec;
        std::filesystem::remove_all(extractTempDir, ec);
    }
};
