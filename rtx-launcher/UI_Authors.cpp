#include "UI_Authors.h"
#include "Globals.h"
#include "imgui.h"

extern void DoOpenRtxGithub();

void RenderUI_Authors() {
    float contentW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;
    
    float topPadding = S(30);
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
    const char* title = L(u8"АВТОРЫ И БЛАГОДАРНОСТИ", "AUTHORS & CREDITS");
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(30)));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", title);
    ImGui::PopFont();


    
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(90)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::BeginChild("AuthorsChild", ImVec2(mainCardSize.x - S(60), mainCardSize.y - S(110)), false);

    ImGui::PushFont(g_imFontHeading);
    ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), "RTX Launcher & Logic");
    ImGui::PopFont();
    ImGui::TextWrapped("%s", L(u8"• Antigravity (Google DeepMind AI) — Разработка архитектуры, дизайна и кода лаунчера", "• Antigravity (Google DeepMind AI) — Architecture, design, and launcher code development"));
    ImGui::TextWrapped("%s", L(u8"• RTX Project — Идея, координация проекта и тестирование", "• RTX Project — Idea, project coordination, and testing"));
    
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::PushFont(g_imFontHeading);
    ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), "RTX Remix Technology");
    ImGui::PopFont();
    ImGui::TextWrapped("%s", L(u8"• NVIDIA — Создание платформы RTX Remix", "• NVIDIA — Creation of the RTX Remix platform"));
    
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::PushFont(g_imFontHeading);
    ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), "Garry's Mod RTX Remix Assets");
    ImGui::PopFont();
    ImGui::TextWrapped("%s", L(u8"• Xenthio — Создатель оригинального лаунчера, бинарных фиксов и ассетов для Garry's Mod", "• Xenthio — Creator of the original launcher, binary fixes, and assets for Garry's Mod"));
    ImGui::TextWrapped("%s", L(u8"• sambow23 — Создатель пропатченной версии RTX Remix", "• sambow23 — Creator of the patched RTX Remix version"));
    ImGui::TextWrapped("%s", L(u8"• Сообщество моддеров — Дополнительные патчи и исправления для стабильной игры", "• Modding Community — Additional patches and fixes for a stable game"));
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::PushFont(g_imFontHeading);
    ImGui::TextColored(ImVec4(0.46f, 0.73f, 0.0f, 1.0f), "%s", L(u8"Open-Source Библиотеки", "Open-Source Libraries"));
    ImGui::PopFont();
    ImGui::TextWrapped("%s", L(u8"• Omar Cornut — Разработка библиотеки Dear ImGui", "• Omar Cornut — Development of the Dear ImGui library"));

    ImGui::EndChild();
    ImGui::PopStyleColor();
}
