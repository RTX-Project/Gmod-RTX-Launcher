#include "UI_Overview.h"
#include "Globals.h"
#include "imgui.h"

extern void CheckLauncherUpdatesAsync();
extern void AppendLog(const std::wstring& line);
extern void RunInBackground(std::function<void()> task);

void RenderUI_Overview() {
    float contentW = ImGui::GetContentRegionAvail().x;
        float availH = ImGui::GetContentRegionAvail().y;
        
        float topPadding = S(30);
        float bottomGridH = S(180);
        float mainCardH = availH - topPadding * 2 - bottomGridH - S(20);
        if (mainCardH < S(200)) mainCardH = S(200);
        
        // --- MAIN CARD ---
        ImGui::SetCursorPos(ImVec2(S(30), topPadding));
        ImVec2 mainCardSize(contentW - S(60), mainCardH);
        ImVec2 mainCardPos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        dl->AddRectFilledMultiColor(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y),
            IM_COL32(12, 28, 24, 255), IM_COL32(12, 28, 24, 255),
            IM_COL32(8, 12, 16, 255), IM_COL32(8, 12, 16, 255));
            
        dl->AddRect(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y), IM_COL32(20, 45, 50, 100), 12.0f, 0, 1.0f);
        
        // --- BUTTON CHECK UPDATES ---
        ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x - S(190), mainCardPos.y + S(15)));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.2f, 0.25f, 1.0f));
        if (ImGui::Button(u8"Проверить обновления", ImVec2(S(175), S(28)))) {
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
            wStatus = g_app.isFirstLaunchMode ? L"Установка..." : L"Подготовка к запуску...";
            statusColor = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
        } else if (isInstalled) {
            wStatus = L"Готов к запуску";
        } else {
            wStatus = L"Требуется установка";
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
        
        float btnW = S(160.0f) + (S(360.0f) - S(160.0f)) * s_transition;
        float btnH = S(45.0f) + (S(12.0f) - S(45.0f)) * s_transition;
        float centerY = startY + S(100.0f) + S(45.0f) / 2.0f;
        ImVec2 btnPos(mainCardPos.x + mainCardSize.x / 2.0f - btnW / 2.0f, centerY - btnH / 2.0f);
        
        bool btnHovered = ImGui::IsMouseHoveringRect(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + btnH));
        bool btnActive = btnHovered && ImGui::IsMouseDown(0);
        
        float rounding = (btnH / 2.0f > 8.0f) ? 8.0f : (btnH / 2.0f);
        
        auto LerpColor = [](ImU32 a, ImU32 b, float t) -> ImU32 {
            int r = (int)(((a >> 0) & 0xFF) + (((b >> 0) & 0xFF) - ((a >> 0) & 0xFF)) * t);
            int g = (int)(((a >> 8) & 0xFF) + (((b >> 8) & 0xFF) - ((a >> 8) & 0xFF)) * t);
            int b_ = (int)(((a >> 16) & 0xFF) + (((b >> 16) & 0xFF) - ((a >> 16) & 0xFF)) * t);
            int a_ = (int)(((a >> 24) & 0xFF) + (((b >> 24) & 0xFF) - ((a >> 24) & 0xFF)) * t);
            return IM_COL32(r, g, b_, a_);
        };
        
        ImU32 colNormal = btnActive ? IM_COL32(0, 200, 100, 255) : (btnHovered ? IM_COL32(0, 255, 150, 255) : IM_COL32(0, 255, 128, 255));
        ImU32 colProgress = IM_COL32(30, 40, 50, 255);
        ImU32 bgCol = LerpColor(colNormal, colProgress, s_transition);
            
        dl->AddRectFilled(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + btnH), bgCol, rounding);
        
        if (s_transition > 0.01f) {
            extern ImVec4 GetAdaptiveProgressColor(float);
            ImVec4 adaptCol = GetAdaptiveProgressColor(g_app.downloadProgressSmooth);
            adaptCol.w *= s_transition; // Fade out the progress fill
            ImU32 fillCol = ImGui::ColorConvertFloat4ToU32(adaptCol);
            
            float fillW = btnW * g_app.downloadProgressSmooth;
            if (fillW > 0.0f) {
                dl->AddRectFilled(btnPos, ImVec2(btnPos.x + fillW, btnPos.y + btnH), fillCol, rounding);
            }
        }
            
        if (s_transition < 0.99f) {
            const char* btnText = isInstalled ? (const char*)u8"► Запустить" : (const char*)u8"Установить";
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
                if (!isInstalled) {
                    GoToWizardStep(WizardStep::Welcome);
                    SwitchPage(Page::InstallerWizard);
                } else {
                    DoLaunchGame();
                }
            }
        }

        // --- BOTTOM GRID ---
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
        
        DrawBottomCard(mainCardPos.x, gridY, "RTX REMIX", (const char*)u8"Включён", true);
        DrawBottomCard(mainCardPos.x + bCardW + S(15), gridY, (const char*)u8"ФИКСЫ", (const char*)u8"Включены", true);
        
        auto GetModVersion = [&](const std::wstring& gamePath) -> std::string {
            if (gamePath.empty()) return "Неизвестно";
            
            std::error_code ec;
            std::string foundVersion = "";
            
            // 1. Ищем mod_version.txt рекурсивно внутри rtx-remix/mods (мод может лежать в подпапке)
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
                                if (!line.empty()) {
                                    return line; // Нашли первую попавшуюся версию мода
                                }
                            }
                        }
                    }
                } catch (...) {}
            }
            
            // 2. Резервный вариант: fixes_version.txt (версия фиксов, скачанная лаунчером)
            fs::path versionFile = fs::path(gamePath) / L"fixes_version.txt";
            if (fs::exists(versionFile, ec)) {
                std::ifstream vf(versionFile);
                std::string localTag;
                if (std::getline(vf, localTag) && !localTag.empty()) {
                    localTag.erase(localTag.find_last_not_of(" \n\r\t") + 1);
                    localTag.erase(0, localTag.find_first_not_of(" \n\r\t"));
                    if (!localTag.empty()) {
                        return localTag;
                    }
                }
            }
            
            return "Неизвестно";
        };
        
        std::string remixVer = "1.5.2";
        std::string modVer = GetModVersion(g_app.installRootPath);
        
        std::string verStr = "Remix: " + remixVer + "\nMod: " + modVer;
        
        // Since we now have 2 lines, we might need a custom draw for the Version card to fit it.
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
        
        DrawVersionCard(mainCardPos.x, gridY + bCardH + S(15), (const char*)u8"ВЕРСИИ", remixVer, modVer);
        
        std::string gpuStr = g_app.gpuInfo.name.empty() ? "NVIDIA RTX" : g_app.gpuInfo.name;
        DrawBottomCard(mainCardPos.x + bCardW + S(15), gridY + bCardH + S(15), "GPU", gpuStr.c_str(), false);
        
        static float g_fiveHourTimer = 0.0f;
        g_fiveHourTimer += ImGui::GetIO().DeltaTime;
        if (g_fiveHourTimer >= 18000.0f) {
            g_fiveHourTimer = 0.0f;
            CheckLauncherUpdatesAsync();
        }
}