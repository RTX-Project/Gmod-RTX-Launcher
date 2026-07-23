// HttpClient.h - обёртка над WinHTTP для:
//   1) GET текстового ответа (JSON API GitHub)
//   2) скачивания файла в поток с колбэком прогресса
// Требует линковки с Winhttp.lib (входит в Windows SDK, доп. установка не нужна).
#pragma once
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <functional>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstdint>

#pragma comment(lib, "winhttp.lib")

class HttpClient {
public:
    // Вспомогательная структура для разбора URL на компоненты (хост, путь, порт, схема)
    struct UrlParts {
        std::wstring host;
        std::wstring path;
        INTERNET_PORT port = 0;
        bool https = false;
    };

    // Парсит строковый URL (вида https://api.github.com/repos/...) с помощью WinHttpCrackUrl
    static UrlParts parseUrl(const std::wstring& url) {
        URL_COMPONENTS uc = {};
        uc.dwStructSize = sizeof(uc);
        wchar_t hostBuf[512] = {};
        wchar_t pathBuf[2048] = {};
        uc.lpszHostName = hostBuf; uc.dwHostNameLength = 512;
        uc.lpszUrlPath = pathBuf; uc.dwUrlPathLength = 2048;
        if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &uc)) {
            throw std::runtime_error("Не удалось разобрать URL");
        }
        UrlParts parts;
        parts.host = hostBuf;
        parts.path = pathBuf;
        parts.port = uc.nPort;
        parts.https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
        return parts;
    }

    // Простой GET, возвращает тело ответа как std::string (UTF-8/ASCII, для JSON достаточно).
    static std::string getText(const std::wstring& initialUrl, const std::wstring& userAgent, const std::wstring& authToken = L"") {
        std::wstring currentUrl = initialUrl;
        int maxRedirects = 8;

        for (int redirectCount = 0; redirectCount < maxRedirects; ++redirectCount) {
            UrlParts u = parseUrl(currentUrl);

            HINTERNET hSession = WinHttpOpen(userAgent.c_str(),
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) throw std::runtime_error("WinHttpOpen failed");

            DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
            WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));

            HINTERNET hConnect = WinHttpConnect(hSession, u.host.c_str(), u.port, 0);
            if (!hConnect) { WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpConnect failed"); }

            DWORD flags = u.https ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", u.path.c_str(),
                nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpOpenRequest failed"); }

            // Ручная обработка перенаправлений для сброса заголовков авторизации на сторонних хостах
            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            std::wstring headers = L"Accept: application/vnd.github+json\r\n";
            if (!authToken.empty() && u.host.find(L"github.com") != std::wstring::npos) {
                headers += L"Authorization: Bearer " + authToken + L"\r\n";
            }
            WinHttpAddRequestHeaders(hRequest, headers.c_str(), (DWORD)headers.size(), WINHTTP_ADDREQ_FLAG_ADD);

            BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
            if (ok) ok = WinHttpReceiveResponse(hRequest, nullptr);

            if (!ok) {
                WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                throw std::runtime_error("HTTP-запрос не удался");
            }

            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

            if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307 || statusCode == 308) {
                wchar_t locBuf[2048] = {};
                DWORD locBufLen = sizeof(locBuf);
                if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
                    locBuf, &locBufLen, WINHTTP_NO_HEADER_INDEX)) {
                    std::wstring newUrl = locBuf;
                    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                    if (newUrl.rfind(L"http://", 0) != 0 && newUrl.rfind(L"https://", 0) != 0) {
                        newUrl = (u.https ? L"https://" : L"http://") + u.host + newUrl;
                    }
                    currentUrl = newUrl;
                    continue;
                }
            }

            if (statusCode != 200) {
                WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                throw std::runtime_error("Ошибка HTTP ответа: " + std::to_string(statusCode));
            }

            std::string result;
            DWORD dwAvail = 0;
            while (WinHttpQueryDataAvailable(hRequest, &dwAvail) && dwAvail > 0) {
                std::string chunk;
                chunk.resize(dwAvail);
                DWORD dwRead = 0;
                if (!WinHttpReadData(hRequest, &chunk[0], dwAvail, &dwRead) || dwRead == 0) break;
                chunk.resize(dwRead);
                result += chunk;
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        throw std::runtime_error("Слишком много перенаправлений (HTTP 302 Loop)");
    }

    // Скачивание бинарного файла на диск с колбэком прогресса (downloaded, total).
    static void downloadFile(const std::wstring& initialUrl, const std::wstring& destPath,
                              const std::wstring& userAgent,
                              std::function<bool(uint64_t, uint64_t)> onProgress /* return false to abort */,
                              const std::wstring& authToken = L"") {
        std::wstring currentUrl = initialUrl;
        int maxRedirects = 8;

        for (int redirectCount = 0; redirectCount < maxRedirects; ++redirectCount) {
            UrlParts u = parseUrl(currentUrl);

            HINTERNET hSession = WinHttpOpen(userAgent.c_str(),
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) throw std::runtime_error("WinHttpOpen failed");

            DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
            WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));

            HINTERNET hConnect = WinHttpConnect(hSession, u.host.c_str(), u.port, 0);
            if (!hConnect) { WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpConnect failed"); }

            DWORD flags = u.https ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", u.path.c_str(),
                nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpOpenRequest failed"); }

            // Ручная обработка редиректов для правильного взаимодействия с Amazon S3 / GitHub CDN
            DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));

            std::wstring headers = L"Accept: application/octet-stream\r\n";
            if (!authToken.empty() && u.host.find(L"github.com") != std::wstring::npos) {
                headers += L"Authorization: Bearer " + authToken + L"\r\n";
            }
            WinHttpAddRequestHeaders(hRequest, headers.c_str(), (DWORD)headers.size(), WINHTTP_ADDREQ_FLAG_ADD);

            BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
            if (ok) ok = WinHttpReceiveResponse(hRequest, nullptr);

            if (!ok) {
                WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                throw std::runtime_error("HTTP-запрос на скачивание не удался");
            }

            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

            if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307 || statusCode == 308) {
                wchar_t locBuf[2048] = {};
                DWORD locBufLen = sizeof(locBuf);
                if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
                    locBuf, &locBufLen, WINHTTP_NO_HEADER_INDEX)) {
                    std::wstring newUrl = locBuf;
                    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                    if (newUrl.rfind(L"http://", 0) != 0 && newUrl.rfind(L"https://", 0) != 0) {
                        newUrl = (u.https ? L"https://" : L"http://") + u.host + newUrl;
                    }
                    currentUrl = newUrl;
                    continue;
                }
            }

            if (statusCode != 200 && statusCode != 206) {
                WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                throw std::runtime_error("Ошибка HTTP скачивания: " + std::to_string(statusCode));
            }

            // Общий размер (если сервер его сообщил).
            uint64_t totalSize = 0;
            {
                wchar_t sizeBuf[64] = {};
                DWORD sizeBufLen = sizeof(sizeBuf);
                if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX,
                                         sizeBuf, &sizeBufLen, WINHTTP_NO_HEADER_INDEX)) {
                    totalSize = _wtoi64(sizeBuf);
                }
            }

            std::ofstream out(destPath.c_str(), std::ios::binary);
            if (!out) {
                WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
                throw std::runtime_error("Не удалось создать файл назначения для сохранения обновления.");
            }

            uint64_t downloaded = 0;
            std::vector<char> buffer(1 << 16);
            DWORD dwAvail = 0;
            bool aborted = false;
            while (WinHttpQueryDataAvailable(hRequest, &dwAvail) && dwAvail > 0) {
                DWORD toRead = dwAvail;
                if (toRead > buffer.size()) toRead = (DWORD)buffer.size();
                DWORD dwRead = 0;
                if (!WinHttpReadData(hRequest, buffer.data(), toRead, &dwRead) || dwRead == 0) break;
                out.write(buffer.data(), dwRead);
                downloaded += dwRead;
                if (onProgress && !onProgress(downloaded, totalSize)) { aborted = true; break; }
            }

            out.close();
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            if (aborted) {
                std::filesystem::remove(destPath);
                throw std::runtime_error("Загрузка отменена пользователем");
            }
            return;
        }

        throw std::runtime_error("Слишком много перенаправлений скачивания (HTTP 302 Loop)");
    }
};
