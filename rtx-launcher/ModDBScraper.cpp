#include "ModDBScraper.h"
#include <wrl.h>
#include <WebView2.h>
#include <thread>
#include <iostream>
#include <fstream>

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
                                    if (url.find(L"/downloads/mirror/") != std::wstring::npos ||
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

                            state->webView->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                    LPWSTR msgStr = nullptr;
                                    if (SUCCEEDED(args->TryGetWebMessageAsString(&msgStr)) && msgStr) {
                                        std::ofstream log(L"moddb_debug.txt", std::ios::app);
                                        log << "[ModDB JS] ";
                                        char utf8[1024];
                                        WideCharToMultiByte(CP_UTF8, 0, msgStr, -1, utf8, 1024, NULL, NULL);
                                        log << utf8 << std::endl;
                                        CoTaskMemFree(msgStr);
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

                                    std::ofstream log(L"moddb_debug.txt", std::ios::app);
                                    char utf8[1024];
                                    WideCharToMultiByte(CP_UTF8, 0, currentUrl.c_str(), -1, utf8, 1024, NULL, NULL);
                                    log << "[ModDB Nav] " << utf8 << std::endl;

                                    std::wstring script = 
                                        L"setInterval(function() {"
                                        L"  if (window.moddbNavigating) return;"
                                        L"  if (document.title.indexOf('Just a moment') != -1 || document.title.indexOf('Cloudflare') != -1 || document.getElementById('challenge-form')) return;"
                                        L"  var log = function(msg) { window.chrome.webview.postMessage(msg); };"
                                        L"  var nav = function(el) { window.moddbNavigating = true; log('Navigating to ' + el.href); if (el.href) window.location.href = el.href; else el.click(); };"
                                        L"  var path = window.location.pathname;"
                                        L"  var url = window.location.href;"
                                        L"  if (path.indexOf('/downloads') == -1 && url.indexOf('moddb.com') != -1) {"
                                        L"    window.moddbNavigating = true; log('Manual nav to mod downloads'); window.location.href = url.split('?')[0] + '/downloads';"
                                        L"  } else if (path.endsWith('/downloads') || path.endsWith('/downloads/')) {"
                                        L"    var el = document.querySelector('.table.downloads .row a.image, .row-content a.image, #downloads a.image');"
                                        L"    if (el) nav(el);"
                                        L"    else {"
                                        L"      var contentArea = document.querySelector('#main, .main, .table.downloads, .content');"
                                        L"      if (contentArea) {"
                                        L"        var links = contentArea.querySelectorAll('a[href*=\"/downloads/\"]');"
                                        L"        for (var i = 0; i < links.length; i++) {"
                                        L"          var h = links[i].href;"
                                        L"          if (h.indexOf('?') == -1 && !h.endsWith('/downloads') && !h.endsWith('/downloads/') && h.indexOf('/downloads/top') == -1 && h.indexOf('/downloads/recently') == -1) {"
                                        L"            window.moddbNavigating = true; log('Found content link: ' + h); window.location.href = h; break;"
                                        L"          }"
                                        L"        }"
                                        L"      }"
                                        L"    }"
                                        L"  } else if (path.indexOf('/downloads/') != -1 && path.indexOf('/start/') == -1) {"
                                        L"    var el = document.querySelector('a#downloadmirrorstoggle') || document.querySelector('a.button-download') || document.querySelector('a[href*=\"/downloads/start/\"]');"
                                        L"    if (el) nav(el); else log('Could not find file download button on ' + url);"
                                        L"  } else if (path.indexOf('/start/') != -1) {"
                                        L"    var el = document.querySelector('p > a[href*=\"/downloads/mirror/\"]') || document.querySelector('a[href*=\"/downloads/mirror/\"]') || document.querySelector('a.button-download');"
                                        L"    if (el) nav(el); else log('Could not find mirror button on ' + url);"
                                        L"  }"
                                        L"}, 1000);";
                                    sender->ExecuteScript(script.c_str(), nullptr);
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

        // Timeout timer
        UINT_PTR timerId = SetTimer(nullptr, 0, 25000, nullptr); // 25 seconds timeout

        // Message loop for this thread
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_TIMER && msg.wParam == timerId) {
                KillTimer(nullptr, timerId);
                if (!state->finished) {
                    state->finished = true;
                    state->onError();
                    PostMessage(state->hwnd, WM_CLOSE, 0, 0);
                }
            }

            if (msg.message == WM_CLOSE) {
                if (!state->finished) {
                    state->finished = true;
                    state->onError();
                }
                break;
            }
        }

        if (state->webViewController) {
            state->webViewController->Close();
        }
        CoUninitialize();
    }).detach();
}
