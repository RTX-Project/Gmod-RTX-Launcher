// ZipExtract.h - распаковка .zip без внешних библиотек, через встроенный
// в Windows COM-объект Shell.Application (тот же механизм, что использует
// проводник для "Извлечь всё"). Требует shell32.lib / ole32.lib / oleaut32.lib
// (все входят в Windows SDK - устанавливать ATL отдельно НЕ нужно, здесь
// используется собственный минимальный smart-pointer вместо CComPtr).
#pragma once
#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shldisp.h>
#include <shellapi.h>
#include <string>
#include <stdexcept>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace detail {
    // Минимальная RAII-обёртка над COM-интерфейсом (замена CComPtr без ATL).
    template <typename T>
    class ComPtr {
    public:
        ComPtr() : p(nullptr) {}
        ~ComPtr() { reset(); }
        ComPtr(const ComPtr&) = delete;
        ComPtr& operator=(const ComPtr&) = delete;

        T** operator&() { reset(); return &p; }
        T* operator->() const { return p; }
        operator T* () const { return p; }
        T* get() const { return p; }

        void reset() { if (p) { p->Release(); p = nullptr; } }

    private:
        T* p;
    };
}

class ZipExtract {
public:
    // Распаковывает ZIP-архив в указанную директорию.
    // Использует встроенный в Windows COM-компонент (Shell.Application), 
    // поэтому не требует внешних библиотек (типа zlib или libzip).
    // Функция блокирует поток, пока все файлы не будут извлечены.
    static void extractAll(const std::wstring& zipPath, const std::wstring& destDir) {
        CreateDirectoryW(destDir.c_str(), nullptr);

        HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        bool needUninit = SUCCEEDED(hrInit) && hrInit != S_FALSE;
        struct ComGuard {
            bool active;
            ~ComGuard() { if (active) CoUninitialize(); }
        } guard{ needUninit };
        if (FAILED(hrInit) && hrInit != RPC_E_CHANGED_MODE) {
            throw std::runtime_error("CoInitializeEx failed");
        }

        detail::ComPtr<IShellDispatch> shell;
        HRESULT hrShell = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
                                            IID_IShellDispatch, (void**)&shell);
        if (FAILED(hrShell) || !shell.get()) throw std::runtime_error("Не удалось создать Shell.Application (COM)");

        VARIANT vZip; VariantInit(&vZip);
        vZip.vt = VT_BSTR; vZip.bstrVal = SysAllocString(zipPath.c_str());

        detail::ComPtr<Folder> zipFolder;
        HRESULT hr1 = shell->NameSpace(vZip, &zipFolder);
        VariantClear(&vZip);
        if (FAILED(hr1) || !zipFolder.get()) throw std::runtime_error("Не удалось открыть zip как папку");

        VARIANT vDest; VariantInit(&vDest);
        vDest.vt = VT_BSTR; vDest.bstrVal = SysAllocString(destDir.c_str());

        detail::ComPtr<Folder> destFolder;
        HRESULT hr2 = shell->NameSpace(vDest, &destFolder);
        VariantClear(&vDest);
        if (FAILED(hr2) || !destFolder.get()) throw std::runtime_error("Не удалось открыть папку назначения");

        detail::ComPtr<FolderItems> items;
        if (FAILED(zipFolder->Items(&items)) || !items.get()) throw std::runtime_error("Не удалось получить содержимое zip");

        long srcCount = 0;
        items->get_Count(&srcCount);

        VARIANT vItems; VariantInit(&vItems);
        vItems.vt = VT_DISPATCH;
        vItems.pdispVal = items.get();
        items.get()->AddRef();

        long options = FOF_NO_UI; // без диалогов прогресса/подтверждений
        VARIANT vOptions; VariantInit(&vOptions);
        vOptions.vt = VT_I4; vOptions.lVal = options;

        HRESULT hrCopy = destFolder->CopyHere(vItems, vOptions);
        VariantClear(&vItems); // освобождает и наш лишний AddRef, и исходную ссылку items

        if (FAILED(hrCopy)) throw std::runtime_error("Ошибка распаковки zip (CopyHere)");

        // ВАЖНО: CopyHere через Shell.Application выполняется асинхронно и может
        // вернуть управление до завершения копирования. Ждём, пока количество
        // объектов в целевой папке не сравняется с количеством в архиве
        // (с разумным таймаутом), периодически перечитывая содержимое папки.
        const int timeoutMs = 5 * 60 * 1000; // 5 минут на большие архивы
        const int stepMs = 250;
        int waited = 0;
        for (;;) {
            detail::ComPtr<Folder> checkFolder;
            VARIANT vDest2; VariantInit(&vDest2);
            vDest2.vt = VT_BSTR; vDest2.bstrVal = SysAllocString(destDir.c_str());
            shell->NameSpace(vDest2, &checkFolder);
            VariantClear(&vDest2);
            if (checkFolder.get()) {
                detail::ComPtr<FolderItems> destItems;
                if (SUCCEEDED(checkFolder->Items(&destItems)) && destItems.get()) {
                    long destCount = 0;
                    destItems->get_Count(&destCount);
                    if (destCount >= srcCount) break;
                }
            }
            if (waited >= timeoutMs) break; // не блокируем навсегда, если что-то пошло не так
            Sleep(stepMs);
            waited += stepMs;
        }
    }

    static void extractAndFlatten(const std::wstring& zipPath, const std::wstring& destDir) {
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        
        std::wstring extractTempDir = std::wstring(tempPath) + L"rtx_launcher_extract_temp_" + std::to_wstring(GetCurrentProcessId()) + L"_" + std::to_wstring(GetTickCount64());
        CreateDirectoryW(extractTempDir.c_str(), nullptr);
        
        // Извлекаем во временную папку
        extractAll(zipPath, extractTempDir);
        
        // Ищем корневую папку внутри (у GitHub релизов обычно одна папка-обертка)
        std::filesystem::path trueRoot = extractTempDir;
        std::vector<std::filesystem::path> children;
        for (const auto& entry : std::filesystem::directory_iterator(extractTempDir)) {
            children.push_back(entry.path());
        }
        if (children.size() == 1 && std::filesystem::is_directory(children[0])) {
            trueRoot = children[0];
        }
        
        // Рекурсивно переносим все файлы
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
                        // КРОМЕ ПАПКИ rtx-remix - ЕЕ ВООБЩЕ НЕ ТРОГАТЬ
                        continue;
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
