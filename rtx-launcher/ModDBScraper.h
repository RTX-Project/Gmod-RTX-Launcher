#pragma once
#include <string>
#include <functional>
#include <windows.h>

class ModDBScraper {
public:
    static void FetchLatestDownloadUrlAsync(const std::wstring& moddbUrl, std::function<void(std::wstring)> onSuccess, std::function<void()> onError);
};
