#include "UI_Settings.h"
#include "Globals.h"
#include "Theme.h"
#include "imgui.h"
#include "LauncherUpdater.h"

extern void SaveSettings();
extern void ShowDiskSelectionModal(void (*onDiskSelected)());
extern void OnDiskChanged();
extern void DoOpenRtxGithub();
extern void CheckLauncherUpdatesAsync();

void RenderUI_Settings() {
    float contentW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;
    
    float topPadding = S(20);
    float mainCardH = availH - topPadding * 2;
    if (mainCardH < S(200)) mainCardH = S(200);
    
    ImGui::SetCursorPos(ImVec2(S(30), topPadding));
    ImVec2 mainCardSize(contentW - S(60), mainCardH);
    ImVec2 mainCardPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    dl->AddRectFilledMultiColor(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y),
        IM_COL32(12, 28, 24, 255), IM_COL32(12, 28, 24, 255),
        IM_COL32(8, 12, 16, 255), IM_COL32(8, 12, 16, 255));
        
    dl->AddRect(mainCardPos, ImVec2(mainCardPos.x + mainCardSize.x, mainCardPos.y + mainCardSize.y), IM_COL32(20, 45, 50, 100), 12.0f, 0, 1.0f);

    ImGui::PushFont(g_imFontTitle);
    const char* title = L(u8"НАСТРОЙКИ", "SETTINGS");
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(20)));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", title);
    ImGui::PopFont();

    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(60)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::BeginChild("SettingsChild", ImVec2(mainCardSize.x - S(60), mainCardSize.y - S(75)), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float rowH = S(85);
    float spacing = S(12);
    float colW = (mainCardSize.x - S(60) - spacing) / 2.0f;
    
    auto DrawCard = [&](ImVec2 pos, ImVec2 size, auto renderContent) {
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 26, 30, 255), S(8.0f));
        dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(40, 45, 50, 255), S(8.0f));
        renderContent(pos, size);
    };

    auto DrawCardAction = [&](const char* id, ImVec2 pos, const char* text, auto onClick) {
        ImGui::SetCursorScreenPos(pos);
        ImGui::PushFont(g_imFontSmall);
        ImVec2 textSize = ImGui::CalcTextSize(text);
        bool clicked = ImGui::InvisibleButton(id, textSize);
        bool hovered = ImGui::IsItemHovered();
        ImGui::SetCursorScreenPos(pos);
        ImGui::TextColored(hovered ? ImVec4(0.0f, 1.0f, 0.5f, 1.0f) : ImVec4(0.0f, 0.90f, 0.46f, 1.0f), "%s", text);
        ImGui::PopFont();
        if (clicked) onClick();
    };

    auto DrawToggle = [&](const char* id, ImVec2 pos, bool active, auto onClick) {
        ImGui::SetCursorScreenPos(pos);
        bool clicked = ImGui::InvisibleButton(id, ImVec2(S(34), S(20)));
        float tRadius = S(10.0f);
        dl->AddRectFilled(pos, ImVec2(pos.x + S(34), pos.y + S(20)), 
            active ? IM_COL32(0, 230, 118, 255) : IM_COL32(60, 60, 60, 255), tRadius);
        dl->AddCircleFilled(ImVec2(pos.x + (active ? S(24) : S(10)), pos.y + S(10)), 
            S(6.0f), IM_COL32(255, 255, 255, 255));
        if (clicked) onClick();
    };

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    
    // ----------------------------------------------------
    // Row 1, Col 1: Game Location
    // ----------------------------------------------------
    DrawCard(p0, ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Расположение игры", "Game Location"));
        ImGui::PopFont();
        
        std::string pathUtf8 = WStringToUTF8(g_app.installRootPath);
        if (pathUtf8.empty()) pathUtf8 = L(u8"Не выбрано", "Not selected");
        
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x - S(40));
        ImGui::Text("%s", pathUtf8.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        DrawCardAction("LocBtn", ImVec2(pos.x + S(20), pos.y + size.y - S(26)), L(u8"СМЕНИТЬ ДИСК →", "CHANGE DRIVE →"), [&]() { ShowDiskSelectionModal(OnDiskChanged); });
    });

    // ----------------------------------------------------
    // Row 1, Col 2: Recovery
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x + colW + spacing, p0.y), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Восстановление", "Recovery"));
        ImGui::PopFont();
        
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x - S(40));
        ImGui::Text("%s", L(u8"Автоматически удалять кэш и лишние файлы при запуске для предотвращения сбоев.", "Automatically delete cache and redundant files on launch to prevent crashes."));
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        DrawToggle("RecTgl", ImVec2(pos.x + size.x - S(50), pos.y + S(14)), g_app.optRepair, [&]() { g_app.optRepair = !g_app.optRepair; SaveSettings(); });
    });

    // ----------------------------------------------------
    // Row 2, Col 1: Launch Mode
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x, p0.y + rowH + spacing), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Режим запуска", "Launch Mode"));
        ImGui::PopFont();
        
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x - S(40));
        if (g_app.launchMode == 2) {
            ImGui::Text("%s", L(u8"Совместимость. Рекомендуется для стабильности на некоторых системах.", "Compatibility. Recommended for stability on some systems."));
        } else {
            ImGui::Text("%s", L(u8"Обычный. Максимальная производительность.", "Normal. Maximum performance."));
        }
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        DrawCardAction("ModeBtn", ImVec2(pos.x + S(20), pos.y + size.y - S(26)), L(u8"ПЕРЕКЛЮЧИТЬ РЕЖИМ →", "SWITCH MODE →"), [&]() { g_app.launchMode = (g_app.launchMode == 2) ? 1 : 2; SaveSettings(); });
    });

    // ----------------------------------------------------
    // Row 2, Col 2: Repository & Fixes
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x + colW + spacing, p0.y + rowH + spacing), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Репозиторий и Фиксы", "Repository & Fixes"));
        ImGui::PopFont();
        
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x - S(40));
        ImGui::Text("%s", L(u8"Открыть страницу проекта на GitHub для скачивания обновлений или сообщений об ошибках.", "Open project page on GitHub to download updates or report issues."));
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        DrawCardAction("GitBtn", ImVec2(pos.x + S(20), pos.y + size.y - S(26)), L(u8"ОТКРЫТЬ В БРАУЗЕРЕ →", "OPEN IN BROWSER →"), [&]() { DoOpenRtxGithub(); });
    });

    // ----------------------------------------------------
    // Row 3, Col 1: Beta Channel
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x, p0.y + (rowH + spacing) * 2), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Бета-канал", "Beta Channel"));
        ImGui::PopFont();
        
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x - S(40));
        ImGui::Text("%s", L(u8"Получать ранние тестовые обновления. Могут содержать ошибки.", "Receive early test updates. May contain bugs."));
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
        
        DrawToggle("BetaTgl", ImVec2(pos.x + size.x - S(50), pos.y + S(14)), g_app.receiveBetaUpdates, [&]() { 
            g_app.receiveBetaUpdates = !g_app.receiveBetaUpdates; 
            LauncherUpdater::INCLUDE_PRERELEASES = g_app.receiveBetaUpdates;
            SaveSettings(); 
            CheckLauncherUpdatesAsync(); 
        });
    });

    // ----------------------------------------------------
    // Row 3, Col 2: GPU Information
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x + colW + spacing, p0.y + (rowH + spacing) * 2), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Видеокарта", "Graphics Card"));
        ImGui::PopFont();
        
        std::string gpuName = g_app.gpuInfo.name.empty() ? L(u8"Не определена", "Not detected") : g_app.gpuInfo.name;
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(32)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(g_app.gpuInfo.isAmdOrIntel ? ImVec4(1.0f, 0.75f, 0.25f, 1.0f) : ImVec4(0.0f, 0.90f, 0.46f, 1.0f), "%s", gpuName.c_str());
        ImVec2 gpuSize = ImGui::CalcTextSize(gpuName.c_str());
        ImGui::PopFont();

        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(52)));
        ImGui::PushFont(g_imFontSmall);
        
        std::string drvStr = std::string(L(u8"Драйвер: ", "Driver: ")) + (g_app.gpuInfo.driverVersionStr.empty() ? L(u8"Неизвестно", "Unknown") : g_app.gpuInfo.driverVersionStr);
        if (!g_app.gpuInfo.driverVersionStr.empty() && g_app.gpuInfo.driverVersion < 596.21f) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 0.9f));
            ImGui::Text("%s %s", drvStr.c_str(), L(u8"(Устарел!)", "(Outdated!)"));
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.55f, 0.9f));
            ImGui::Text("%s", drvStr.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(68)));
        if (g_app.gpuInfo.isFrankenstein) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
            ImGui::Text("%s", L(u8"Boosty: Требуется подписка", "Boosty: Subscription required"));
            ImGui::PopStyleColor();
        } else if (g_app.gpuInfo.isAmdOrIntel) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.25f, 0.9f));
            ImGui::Text("%s", L(u8"(Совместимость не гарантируется)", "(Compatibility not guaranteed)"));
            ImGui::PopStyleColor();
        } else if (g_app.gpuInfo.isNvidiaRtx) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.90f, 0.46f, 0.9f));
            ImGui::Text("%s", L(u8"(Полная аппаратная поддержка)", "(Full hardware support)"));
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();
    });

    // ----------------------------------------------------
    // Row 4, Col 1: Language
    // ----------------------------------------------------
    DrawCard(ImVec2(p0.x, p0.y + (rowH + spacing) * 3), ImVec2(colW, rowH), [&](ImVec2 pos, ImVec2 size) {
        ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(12)));
        ImGui::PushFont(g_imFontRegular);
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", L(u8"Язык / Language", "Language / Язык"));
        ImGui::PopFont();
        
                ImGui::SetCursorScreenPos(ImVec2(pos.x + S(20), pos.y + S(36)));
        ImGui::PushFont(g_imFontSmall);
        
        bool isRu = (g_app.language != "en");
        bool isEn = (g_app.language == "en");

        auto DrawLangOption = [&](const char* label, bool active, ImVec2 cPos, auto onClick) {
            ImVec2 tSize = ImGui::CalcTextSize(label);
            ImVec2 boxSize(S(100), S(32)); 
            ImGui::SetCursorScreenPos(cPos);
            bool clicked = ImGui::InvisibleButton(label, boxSize);
            bool hovered = ImGui::IsItemHovered();
            
            if (active) {
                dl->AddRectFilled(cPos, ImVec2(cPos.x + boxSize.x, cPos.y + boxSize.y), IM_COL32(32, 38, 48, 255), S(4.0f));
            } else if (hovered) {
                dl->AddRectFilled(cPos, ImVec2(cPos.x + boxSize.x, cPos.y + boxSize.y), IM_COL32(24, 28, 36, 255), S(4.0f));
            }
            
            ImVec2 textPos(cPos.x + S(16), cPos.y + (boxSize.y - tSize.y) / 2.0f);
            ImGui::SetCursorScreenPos(textPos);
            ImGui::TextColored(active ? ImVec4(1,1,1,1) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", label);
            if (clicked && !active) onClick();
        };

        DrawLangOption(u8"Русский", isRu, ImVec2(pos.x + S(20), pos.y + S(40)), [&]() {
            g_app.language = "ru"; SaveSettings();
        });
        
        DrawLangOption("English", isEn, ImVec2(pos.x + S(130), pos.y + S(40)), [&]() {
            g_app.language = "en"; SaveSettings();
        });
        
        ImGui::PopFont();

    });

    ImGui::EndChild();
    ImGui::PopStyleColor();
}