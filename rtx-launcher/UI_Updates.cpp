#include "UI_Updates.h"
#include "Globals.h"
#include "imgui.h"
#include <functional>
#include <algorithm>
#undef min
#undef max

extern void CheckLauncherUpdatesAsync();
extern void AppendLog(const std::wstring& line);
extern void RunInBackground(std::function<void()> task);

void RenderUI_Updates() {
        ImGui::PushFont(g_imFontTitle);
        ImGui::Text(u8"ОБНОВЛЕНИЯ И ЧЕЙНДЖЛОГ");
        ImGui::PopFont();

        ImGui::SetCursorPos(S(40, 90));
        if (ImGui::Button(u8"Назад", S(90, 32))) SwitchPage(Page::Overview);

        ImGui::SameLine(S(140.0f));
        if (ImGui::Button(u8"Проверить обновления", S(180, 32))) {
            CheckLauncherUpdatesAsync();
        }

        ImGui::SetCursorPos(S(40, 132));
        ImGui::BeginChild("UpdatesMainChild", S(780, 345), true);

        ImGui::PushFont(g_imFontHeading);
        ImGui::TextColored(ImVec4(0.0f, 0.90f, 0.46f, 1.0f), u8"● ТЕКУЩАЯ СБОРКА: Build %d (Версия v%s)",
            LauncherUpdater::CURRENT_BUILD_NUMBER, WStringToUTF8(LauncherUpdater::CURRENT_VERSION).c_str());
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (g_launcherUpdateInfo.hasUpdate) {
            std::string updateHeader = u8"Доступно новое обновление: v" + WStringToUTF8(g_launcherUpdateInfo.version);
            if (g_launcherUpdateInfo.buildNumber > 0) {
                updateHeader += u8" (Сборка #" + std::to_string(g_launcherUpdateInfo.buildNumber) + ")";
            }
            ImGui::PushFont(g_imFontHeading);
            ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.25f, 1.0f), "⭐ %s", updateHeader.c_str());
            ImGui::PopFont();
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"У вас установлена самая свежая версия лаунчера.");
        }

        ImGui::Spacing();
        ImGui::TextDisabled(u8"ОПИСАНИЕ ИЗМЕНЕНИЙ (CHANGELOG С GITHUB):");

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));
        ImGui::PushFont(g_imFontSmall);
        std::string pageNotes = WStringToUTF8(g_launcherUpdateInfo.releaseNotes);
        if (pageNotes.empty()) {
            if (g_isCheckingLauncherUpdate.load()) {
                pageNotes = u8"Загрузка списка изменений с GitHub...";
            } else {
                pageNotes = u8"Для данной сборки нет дополнительного текста описания в релизе GitHub.";
            }
        }
        
        ImVec2 textSize = ImGui::CalcTextSize(pageNotes.c_str(), nullptr, false, S(740.0f));
        float childHeight = textSize.y + S(16.0f);
        if (childHeight > S(150.0f)) childHeight = S(150.0f);
        if (childHeight < S(36.0f)) childHeight = S(36.0f);

        ImGui::BeginChild("ChangelogPageScroll", ImVec2(S(760.0f), childHeight), true, ImGuiWindowFlags_NoScrollWithMouse);
        
        static float targetScrollY = 0.0f;
        static float currentScrollY = 0.0f;

        float actualScrollY = ImGui::GetScrollY();
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel == 0.0f && std::abs(actualScrollY - currentScrollY) > 2.0f) {
            targetScrollY = actualScrollY;
            currentScrollY = actualScrollY;
        }

        if (ImGui::IsWindowHovered() && wheel != 0.0f) {
            targetScrollY -= wheel * 100.0f;
        }

        float maxScroll = ImGui::GetScrollMaxY();
        if (targetScrollY < 0.0f) targetScrollY = 0.0f;
        if (targetScrollY > maxScroll) targetScrollY = maxScroll;

        if (std::abs(targetScrollY - currentScrollY) > 0.5f) {
            g_isSmoothScrolling = true;
        }

        float lerpT = std::min(10.0f * ImGui::GetIO().DeltaTime, 0.5f);
        currentScrollY += (targetScrollY - currentScrollY) * lerpT;
        ImGui::SetScrollY(currentScrollY);

        ImGui::TextWrapped("%s", pageNotes.c_str());
        ImGui::EndChild();
        ImGui::PopFont();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        if (g_launcherUpdateInfo.hasUpdate) {
            PushAccentButton();
            if (ImGui::Button(u8"СКАЧАТЬ И УСТАНОВИТЬ ОБНОВЛЕНИЕ", S(320, 40))) {
                g_app.isDownloading = true;
                RunInBackground([]() {
                    LauncherUpdater::DownloadAndApplyUpdate(g_launcherUpdateInfo.downloadUrl,
                        [](const std::wstring& msg) { AppendLog(msg); },
                        [](float p) {
                            g_app.downloadProgress = p;
                            { std::lock_guard<std::mutex> lock(g_app.statsMutex); g_app.downloadTitleText = L"Загрузка обновления..."; }
                        },
                        g_app.githubToken);
                });
            }
            PopAccentButton();

            if (g_app.isDownloading.load()) {
                ImGui::Spacing();
                std::wstring stats;
                { std::lock_guard<std::mutex> lock(g_app.statsMutex); stats = g_app.downloadTitleText; }
                ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.25f, 1.0f), "%s %.0f%%", WStringToUTF8(stats).c_str(), g_app.downloadProgress * 100.0f);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, GetAdaptiveProgressColor(g_app.downloadProgress));
                ImGui::ProgressBar(g_app.downloadProgress, S(760, 15), "");
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndChild();
    }