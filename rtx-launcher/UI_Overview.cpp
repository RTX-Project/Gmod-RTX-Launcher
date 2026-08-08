#include "UI_Overview.h"
#include "Globals.h"
#include "imgui.h"

extern void CheckLauncherUpdatesAsync();
extern void AppendLog(const std::wstring& line);
extern void RunInBackground(std::function<void()> task);


static std::string GetModVersion(const std::wstring& gamePath) {
    if (gamePath.empty()) return L(u8"Неизвестно", "Unknown");
    
    std::error_code ec;
    
    // 1. Ищем mod_version.txt рекурсивно внутри rtx-remix/mods
    fs::path modsPath = fs::path(gamePath) / L"rtx-remix" / L"mods";
    if (fs::exists(modsPath, ec)) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(modsPath, fs::directory_options::skip_permission_denied, ec)) {
                if (ec || !entry.is_regular_file(ec)) continue;
                if (entry.path().filename() == L"mod_version.txt") {
                    std::ifstream vf(entry.path());
                    std::string line;
                    if (std::getline(vf, line) && !line.empty()) {
                        line.erase(line.find_last_not_of(" \n\r\t") + 1);
                        line.erase(0, line.find_first_not_of(" \n\r\t"));
                        if (!line.empty()) return line;
                    }
                }
            }
        } catch (...) {}
    }
    
    // 2. Резервный вариант: fixes_version.txt
    fs::path versionFile = fs::path(gamePath) / L"fixes_version.txt";
    if (fs::exists(versionFile, ec)) {
        std::ifstream vf(versionFile);
        std::string localTag;
        if (std::getline(vf, localTag) && !localTag.empty()) {
            localTag.erase(localTag.find_last_not_of(" \n\r\t") + 1);
            localTag.erase(0, localTag.find_first_not_of(" \n\r\t"));
            if (!localTag.empty()) return localTag;
        }
    }
    return L(u8"Неизвестно", "Unknown");
}

static void RenderOverview_MainCard(float contentW, float topPadding, float mainCardH, ImVec2& mainCardPos, ImVec2& mainCardSize) {
    ImGui::SetCursorPos(ImVec2(S(30), topPadding));
    mainCardSize = ImVec2(contentW - S(60), mainCardH);
    mainCardPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    dl->AddRectFilledMultiColor(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y),
        IM_COL32(12, 28, 24, 255), IM_COL32(12, 28, 24, 255),
        IM_COL32(8, 12, 16, 255), IM_COL32(8, 12, 16, 255));
        
    dl->AddRect(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y), IM_COL32(20, 45, 50, 100), 12.0f, 0, 1.0f);
    
    // --- BUTTON CHECK UPDATES ---
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x - S(190), mainCardPos.y + S(15)));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.2f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.2f, 0.25f, 1.0f));
    if (ImGui::Button(L(u8"Проверить обновления", "Check for updates"), ImVec2(S(175), S(28)))) {
        CheckLauncherUpdatesAsync();
    }
    ImGui::PopStyleColor(2);
    
    float startY = mainCardPos.y + mainCardSize.y / 2.0f - S(65.0f);
    
    ImGui::PushFont(g_imFontTitle);
    const char* title = "Garry's Mod + RTX";
    ImVec2 titleSize = ImGui::CalcTextSize(title);
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x / 2.0f - titleSize.x / 2.0f, startY));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", title);
    ImGui::PopFont();
    
    ImGui::PushFont(g_imFontHeading);
    bool isRunning = g_app.operationRunning.load();
    bool isInstalled = !g_app.installRootPath.empty();
    
    std::wstring wStatus;
    ImVec4 statusColor = ImVec4(0.00f, 0.90f, 0.46f, 1.0f);
    
    if (isRunning || g_app.isDownloading) {
        if (g_launcherUpdateInfo.hasUpdate && g_app.isDownloading) {
            wStatus = L(L"Обновление лаунчера...", L"Updating launcher...");
        } else {
            wStatus = g_app.isFirstLaunchMode ? L(L"Установка...", L"Installing...") : L(L"Подготовка к запуску...", L"Preparing to launch...");
        }
        statusColor = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
    } else if (isInstalled) {
        wStatus = L(L"Готов к запуску", L"Ready to launch");
    } else {
        wStatus = L(L"Требуется установка", L"Installation required");
        statusColor = ImVec4(1.0f, 0.4f, 0.2f, 1.0f);
    }
    
    std::string status = WStringToUTF8(wStatus);
    ImVec2 statusSize = ImGui::CalcTextSize(status.c_str());
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x / 2.0f - statusSize.x / 2.0f, startY + S(55.0f)));
    ImGui::TextColored(statusColor, "%s", status.c_str());
    ImGui::PopFont();
    
    bool isDownloading = (isRunning || g_app.isDownloading);
    
    static float s_transition = 0.0f; // 0.0 = Button, 1.0 = Progress bar
    float targetTransition = isDownloading ? 1.0f : 0.0f;
    float dt = ImGui::GetIO().DeltaTime * 10.0f;
    if (dt > 1.0f) dt = 1.0f;
    s_transition += (targetTransition - s_transition) * dt;
    if (s_transition < 0.0f) s_transition = 0.0f;
    if (s_transition > 1.0f) s_transition = 1.0f;
    
    float btnW = S(160.0f) + (S(360.0f) - S(160.0f)) * s_transition;
    float btnH = S(45.0f) + (S(12.0f) - S(45.0f)) * s_transition;
    float centerY = startY + S(100.0f) + S(45.0f) / 2.0f;
    ImVec2 btnPos(mainCardPos.x + mainCardSize.x / 2.0f - btnW / 2.0f, centerY - btnH / 2.0f);
    
    bool btnHovered = ImGui::IsMouseHoveringRect(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + btnH));
    bool btnActive = btnHovered && ImGui::IsMouseDown(0);
    
    float rounding = (btnH / 2.0f > 8.0f) ? 8.0f : (btnH / 2.0f);
    
    auto LerpColor = [](ImU32 a, ImU32 b, float t) -> ImU32 {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        int r = (int)(((a >> 0) & 0xFF) + (((b >> 0) & 0xFF) - ((a >> 0) & 0xFF)) * t);
        int g = (int)(((a >> 8) & 0xFF) + (((b >> 8) & 0xFF) - ((a >> 8) & 0xFF)) * t);
        int b_ = (int)(((a >> 16) & 0xFF) + (((b >> 16) & 0xFF) - ((a >> 16) & 0xFF)) * t);
        int a_ = (int)(((a >> 24) & 0xFF) + (((b >> 24) & 0xFF) - ((a >> 24) & 0xFF)) * t);
        
        if (r < 0) r = 0; if (r > 255) r = 255;
        if (g < 0) g = 0; if (g > 255) g = 255;
        if (b_ < 0) b_ = 0; if (b_ > 255) b_ = 255;
        if (a_ < 0) a_ = 0; if (a_ > 255) a_ = 255;
        
        return IM_COL32(r, g, b_, a_);
    };
    
    ImU32 colNormal = btnActive ? IM_COL32(0, 200, 100, 255) : (btnHovered ? IM_COL32(0, 255, 150, 255) : IM_COL32(0, 255, 128, 255));
    
    std::string errCheck = WStringToUTF8(g_app.downloadStatsText);
    bool isError = (errCheck.find("Ошибка") != std::string::npos) || (errCheck.find("error") != std::string::npos);
    if (isError && !g_app.isDownloading) {
        colNormal = btnActive ? IM_COL32(200, 50, 50, 255) : (btnHovered ? IM_COL32(255, 50, 50, 255) : IM_COL32(220, 40, 40, 255));
    }
    
    ImU32 colProgress = IM_COL32(30, 40, 50, 255);
    ImU32 bgCol = LerpColor(colNormal, colProgress, s_transition);
        
    dl->AddRectFilled(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + btnH), bgCol, rounding);
    
    if (s_transition > 0.01f) {
        extern ImVec4 GetAdaptiveProgressColor(float);
        ImVec4 adaptCol = GetAdaptiveProgressColor(g_app.downloadProgressSmooth);
        if (isError) adaptCol = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        adaptCol.w *= s_transition;
        ImU32 fillCol = ImGui::ColorConvertFloat4ToU32(adaptCol);
        
        float fillW = btnW * g_app.downloadProgressSmooth;
        if (fillW > 0.0f) {
            dl->PushClipRect(btnPos, ImVec2(btnPos.x + fillW, btnPos.y + btnH), true);
            dl->AddRectFilled(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + btnH), fillCol, rounding);
            dl->PopClipRect();
        }
    }
        
    if (s_transition < 0.99f) {
        const char* btnText;
        if (g_launcherUpdateInfo.hasUpdate) {
            btnText = L(u8"Обновить лаунчер", "Update launcher");
        } else {
            btnText = isInstalled ? L(u8"► Запустить", "► Launch") : L(u8"Установить", "Install");
        }
        ImGui::PushFont(g_imFontHeading);
        ImVec2 textSz = ImGui::CalcTextSize(btnText);
        ImGui::SetCursorScreenPos(ImVec2(btnPos.x + btnW/2.0f - textSz.x/2.0f, btnPos.y + btnH/2.0f - textSz.y/2.0f));
        ImGui::TextColored(ImVec4(0, 0, 0, 1.0f - s_transition), "%s", btnText);
        ImGui::PopFont();
    }
    
    if (s_transition > 0.01f) {
        std::string statsUtf8 = WStringToUTF8(g_app.downloadStatsText);
        ImGui::PushFont(g_imFontSmall);
        ImVec2 stSz = ImGui::CalcTextSize(statsUtf8.c_str());
        ImGui::SetCursorScreenPos(ImVec2(btnPos.x + btnW/2.0f - stSz.x/2.0f, btnPos.y + btnH + S(12.0f)));
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, s_transition), "%s", statsUtf8.c_str());
        ImGui::PopFont();
    }
    
    ImGui::SetCursorScreenPos(btnPos);
    if (ImGui::InvisibleButton("PlayBtn", ImVec2(btnW, btnH))) {
        if (!isDownloading) {
            if (g_launcherUpdateInfo.hasUpdate) {
                g_app.isDownloading = true;
                RunInBackground([]() {
                    LauncherUpdater::DownloadAndApplyUpdate(g_launcherUpdateInfo.downloadUrl,
                        g_launcherUpdateInfo.releaseNotes,
                        [](const std::wstring& msg) { AppendLog(msg); },
                        [](float p) {
                            g_app.downloadProgress = p;
                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L(L"Загрузка обновления...", L"Downloading update..."); }
                        },
                        g_app.githubToken);
                });
            } else if (!isInstalled) {
                GoToWizardStep(WizardStep::Welcome);
                SwitchPage(Page::InstallerWizard);
            } else {
                DoLaunchGame();
            }
        }
    }
}

static void RenderOverview_BottomGrid(float contentW, ImVec2 mainCardPos, ImVec2 mainCardSize, float bottomGridH) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float gridY = mainCardPos.y + mainCardSize.y + S(20);
    float bCardW = (contentW - S(60) - S(15)) / 2.0f;
    float bCardH = (bottomGridH - S(15)) / 2.0f;
    
    auto DrawBottomCard = [&](float x, float y, const char* title, const char* value, bool isGreen) {
        ImVec2 pos(x, y);
        dl->AddRectFilled(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(15, 20, 25, 255), 8.0f);
        dl->AddRect(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(25, 30, 40, 255), 8.0f, 0, 1.0f);
        
        ImGui::PushFont(g_imFontSmall);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(15)));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "%s", title);
        ImGui::PopFont();
        
        ImGui::PushFont(g_imFontHeading);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(40)));
        if (isGreen) ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), "%s", value);
        else ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", value);
        ImGui::PopFont();
    };
    
    DrawBottomCard(mainCardPos.x, gridY, "RTX REMIX", L(u8"Включён", "Enabled"), true);
    DrawBottomCard(mainCardPos.x + bCardW + S(15), gridY, L(u8"ФИКСЫ", "FIXES"), L(u8"Включены", "Enabled"), true);
    
    std::string remixVer = "1.5.2";
    std::string modVer = GetModVersion(g_app.installRootPath);
    
    auto DrawVersionCard = [&](float x, float y, const char* title, const std::string& remix, const std::string& mod) {
        ImVec2 pos(x, y);
        dl->AddRectFilled(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(15, 20, 25, 255), 8.0f);
        dl->AddRect(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(25, 30, 40, 255), 8.0f, 0, 1.0f);
        
        ImGui::PushFont(g_imFontSmall);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(12)));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "%s", title);
        ImGui::PopFont();
        
        ImGui::PushFont(g_imFontRegular);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(35)));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Remix: ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), "%s", remix.c_str());
        
        if (g_app.hasLaunchedGame) {
            ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(55)));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Mod: ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), "%s", mod.c_str());
        }
        ImGui::PopFont();
    };
    
    DrawVersionCard(mainCardPos.x, gridY + bCardH + S(15), L(u8"ВЕРСИИ", "VERSIONS"), remixVer, modVer);
    
    std::string gpuStr = g_app.gpuInfo.name.empty() ? "NVIDIA RTX" : g_app.gpuInfo.name;
    
    {
        float x = mainCardPos.x + bCardW + S(15);
        float y = gridY + bCardH + S(15);
        ImVec2 pos(x, y);
        dl->AddRectFilled(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(15, 20, 25, 255), 8.0f);
        dl->AddRect(pos, ImVec2(pos.x + bCardW, pos.y + bCardH), IM_COL32(25, 30, 40, 255), 8.0f, 0, 1.0f);
        
        ImGui::PushFont(g_imFontSmall);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(12)));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "GPU");
        ImGui::PopFont();
        
        ImGui::PushFont(g_imFontRegular);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(35)));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", gpuStr.c_str());
        ImGui::PopFont();
        
        ImGui::PushFont(g_imFontSmall);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(55)));
        
        std::string drvStr = std::string(L(u8"Драйвер: ", "Driver: ")) + (g_app.gpuInfo.driverVersionStr.empty() ? L(u8"Неизвестно", "Unknown") : g_app.gpuInfo.driverVersionStr);
        if (!g_app.gpuInfo.driverVersionStr.empty()) {
            if (g_app.gpuInfo.driverVersion < 596.21f) {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s %s", drvStr.c_str(), L(u8"(Устарел!)", "(Outdated!)"));
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), "%s (OK)", drvStr.c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "%s", drvStr.c_str());
        }
        
        if (g_app.gpuInfo.isFrankenstein) {
            ImGui::SetCursorScreenPos(ImVec2(pos.x + S(15), pos.y + S(70)));
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "%s", L(u8"Boosty: Требуется подписка", "Boosty: Subscription required"));
        }
        ImGui::PopFont();
    }
}

void RenderUI_Overview() {
    float contentW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;
    
    float topPadding = S(30);
    float bottomGridH = S(180);
    float mainCardH = availH - topPadding * 2 - bottomGridH - S(20);
    if (mainCardH < S(200)) mainCardH = S(200);
    
    ImVec2 mainCardPos, mainCardSize;
    RenderOverview_MainCard(contentW, topPadding, mainCardH, mainCardPos, mainCardSize);
    RenderOverview_BottomGrid(contentW, mainCardPos, mainCardSize, bottomGridH);

    static float g_fiveHourTimer = 0.0f;
    g_fiveHourTimer += ImGui::GetIO().DeltaTime;
    if (g_fiveHourTimer >= 18000.0f) {
        g_fiveHourTimer = 0.0f;
        CheckLauncherUpdatesAsync();
    }
}
