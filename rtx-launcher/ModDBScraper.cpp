#include "ModDBScraper.h"
#include <wrl.h>
#include <WebView2.h>
#include <thread>
#include <iostream>

using namespace Microsoft::WRL;

// Hidden window class for WebView2 parent
const wchar_t* kScraperWindowClassName = L"ModDBScraperHiddenWindow";

struct ScraperState {
    HWND hwnd = nullptr;
    ComPtr<ICoreWebView2Controller> webViewController;
    ComPtr<ICoreWebView2> webView;
    std::wstring moddbUrl;
    std::function<void(std::wstring)> onSuccess;
    std::function<void()> onError;
    bool finished = false;
};

static LRESULT CALLBACK ScraperWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ModDBScraper::FetchLatestDownloadUrlAsync(const std::wstring& moddbUrl, std::function<void(std::wstring)> onSuccess, std::function<void()> onError) {
    std::thread([moddbUrl, onSuccess, onError]() {
        // Initialize COM
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.lpfnWndProc = ScraperWndProc;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.lpszClassName = kScraperWindowClassName;
        RegisterClassEx(&wcex);

        HWND hwnd = CreateWindowEx(0, kScraperWindowClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), nullptr);

        auto state = std::make_shared<ScraperState>();
        state->hwnd = hwnd;
        state->moddbUrl = moddbUrl;
        state->onSuccess = onSuccess;
        state->onError = onError;

        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [state](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                    if (FAILED(result) || !env) {
                        state->onError();
                        state->finished = true;
                        return S_OK;
                    }

                    env->CreateCoreWebView2Controller(state->hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [state](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result) || !controller) {
                                state->onError();
                                state->finished = true;
                                return S_OK;
                            }

                            state->webViewController = controller;
                            state->webViewController->get_CoreWebView2(&state->webView);

                            // Disable popup windows
                            ComPtr<ICoreWebView2Settings> settings;
                            if (SUCCEEDED(state->webView->get_Settings(&settings))) {
                                settings->put_AreDefaultScriptDialogsEnabled(FALSE);
                                settings->put_IsScriptEnabled(TRUE);
                            }

                            // Intercept Navigation to file downloads
                            EventRegistrationToken token;
                            state->webView->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                                [state](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                                    LPWSTR uriStr = nullptr;
                                    args->get_Uri(&uriStr);
                                    std::wstring url = uriStr ? uriStr : L"";
                                    if (uriStr) CoTaskMemFree(uriStr);
                                    
                                    // If we hit a mirror link or actual file download
                                    if (url.find(L"moddb.com/downloads/mirror/") != std::wstring::npos ||
                                        url.find(L".zip") != std::wstring::npos) {
                                        
                                        args->put_Cancel(TRUE); // Cancel navigation to prevent actual download in webview
                                        if (!state->finished) {
                                            state->finished = true;
                                            state->onSuccess(url);
                                            PostMessage(state->hwnd, WM_CLOSE, 0, 0);
                                        }
                                    }
                                    return S_OK;
                                }).Get(), &token);

                            // Inject JS on navigation complete
                            state->webView->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                [state](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                    LPWSTR uriStr = nullptr;
                                    sender->get_Source(&uriStr);
                                    std::wstring currentUrl = uriStr ? uriStr : L"";
                                    if (uriStr) CoTaskMemFree(uriStr);

                                    if (currentUrl.find(L"/downloads") == std::wstring::npos && currentUrl.find(L"moddb.com") != std::wstring::npos) {
                                        // We are on the main mod page, redirect to downloads page
                                        std::wstring script = L"window.location.href = window.location.href.split('?')[0] + '/downloads';";
                                        sender->ExecuteScript(script.c_str(), nullptr);
                                    } else if (currentUrl.find(L"/downloads") != std::wstring::npos && currentUrl.find(L"/start/") == std::wstring::npos) {
                                        // On the downloads page, find the latest file link
                                        std::wstring script = L"var el = document.querySelector('.table.table.downloads .row a.image'); if(el) window.location.href = el.href;";
                                        sender->ExecuteScript(script.c_str(), nullptr);
                                    } else if (currentUrl.find(L"/start/") != std::wstring::npos) {
                                        // On the start page, click the download button
                                        std::wstring script = L"var el = document.querySelector('a.button.button-download'); if(el) window.location.href = el.href;";
                                        sender->ExecuteScript(script.c_str(), nullptr);
                                    }
                                    return S_OK;
                                }).Get(), &token);

                            // Start navigation
                            state->webView->Navigate(state->moddbUrl.c_str());

                            return S_OK;
                        }).Get());
                    return S_OK;
                }).Get());

        if (FAILED(hr)) {
            state->onError();
            state->finished = true;
        }

        // Message loop for this thread
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (state->finished && msg.message == WM_CLOSE) {
                break;
            }
        }

        if (state->webViewController) {
            state->webViewController->Close();
        }
        CoUninitialize();
    }).detach();
}
