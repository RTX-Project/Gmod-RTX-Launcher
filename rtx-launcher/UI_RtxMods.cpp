#include "UI_RtxMods.h"
#include "Globals.h"
#include "imgui.h"
#include "ModDBScraper.h"

extern void LoadRtxReleases();

void RenderUI_RtxMods() {
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
    const char* title = (const char*)u8"METROSTROI RTX";
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(30)));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", title);
    ImGui::PopFont();

    ImGui::PushFont(g_imFontRegular);
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x - S(210) * 2, mainCardPos.y + S(30)));
    // ModDB download is now handled during Launch Game
    
    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + mainCardSize.x - S(210), mainCardPos.y + S(30)));
    if (ImGui::Button((const char*)u8"Обновить релизы", ImVec2(S(180), S(35)))) LoadRtxReleases();
    ImGui::PopFont();
    
    auto drawCard = [](const char* id, const char* title, const char* subtitle, bool selected, ImU32 imgColor, bool isBeta, const char* desc, bool disabled) {
        ImVec2 size = S(235, 280);
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        bool clicked = false;
        if (!disabled) {
            clicked = ImGui::InvisibleButton(id, size);
        } else {
            ImGui::InvisibleButton(id, size);
        }
        
        bool hovered = !disabled && ImGui::IsItemHovered();
        bool active = !disabled && ImGui::IsItemActive();
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        float rounding = 12.0f;
        ImU32 bgColor = IM_COL32(28, 30, 36, 255);
        if (hovered) bgColor = IM_COL32(35, 38, 45, 255);
        if (active) bgColor = IM_COL32(20, 22, 26, 255);
        
        if (disabled) {
            bgColor = IM_COL32(20, 22, 26, 180);
            imgColor = IM_COL32(40, 45, 50, 255);
        }
        
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), bgColor, rounding);
        
        float imgHeight = size.y * 0.45f;
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + imgHeight), imgColor, rounding, ImDrawFlags_RoundCornersTop);
        
        drawList->AddRectFilledMultiColor(p, ImVec2(p.x + size.x, p.y + imgHeight), IM_COL32(0,0,0,0), IM_COL32(0,0,0,0), IM_COL32(0,0,0,180), IM_COL32(0,0,0,180));
        
        if (selected && !disabled) {
            drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(255, 200, 0, 255), rounding, 0, 2.0f);
        } else if (hovered) {
            drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(100, 100, 100, 150), rounding, 0, 1.0f);
        }

        ImGui::PushFont(g_imFontRegular);
        float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
        float badgePaddingX = 8.0f;
        float badgePaddingY = 4.0f;
        ImVec2 badgeRectMin = ImVec2(p.x + size.x - 16.0f - subtitleWidth - badgePaddingX * 2.0f, p.y + 16.0f);
        ImVec2 badgeRectMax = ImVec2(p.x + size.x - 16.0f, p.y + 16.0f + ImGui::GetFontSize() + badgePaddingY * 2.0f);
        
        ImU32 badgeCol = isBeta ? IM_COL32(255, 100, 0, 255) : IM_COL32(0, 200, 100, 255);
        if (disabled) badgeCol = IM_COL32(150, 150, 150, 255);
        
        ImU32 badgeBgCol = IM_COL32(0, 0, 0, 160);
        
        drawList->AddRectFilled(badgeRectMin, badgeRectMax, badgeBgCol, 6.0f);
        drawList->AddRect(badgeRectMin, badgeRectMax, badgeCol, 6.0f, 0, 1.5f);
        
        ImVec2 badgeTextP = ImVec2(badgeRectMin.x + badgePaddingX, badgeRectMin.y + badgePaddingY);
        drawList->AddText(badgeTextP, badgeCol, subtitle);

        ImVec2 textP = ImVec2(p.x + 16.0f, p.y + imgHeight + 16.0f);
        drawList->AddText(textP, disabled ? IM_COL32(150, 150, 150, 255) : IM_COL32(255, 255, 255, 255), title);
        ImGui::PopFont();
        
        ImVec2 descP = ImVec2(p.x + 16.0f, p.y + imgHeight + 45.0f);
        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), descP, IM_COL32(140, 140, 140, 255), desc, NULL, size.x - 32.0f);
        
        return clicked;
    };

    ImGui::SetCursorScreenPos(ImVec2(mainCardPos.x + S(30), mainCardPos.y + S(90)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    
    ImGui::BeginChild("ModsChild", ImVec2(mainCardSize.x - S(60), mainCardSize.y - S(110)), false, ImGuiWindowFlags_NoScrollbar);
    
    if (drawCard("Mode0", u8"Оригинал + Освещение", u8"Бета", g_app.rtxSelectedIndex == 0, IM_COL32(180, 80, 20, 255), true, u8"Оригинальные материалы с улучшенным освещением.\nВозможны графические баги.", false)) {
        g_app.rtxSelectedIndex = 0;
    }
    ImGui::SameLine(S(255));
    if (drawCard("Mode1", u8"Улучшенные текстуры", u8"Metrostroi_full", g_app.rtxSelectedIndex == 1, IM_COL32(20, 80, 180, 255), false, u8"Замена оригинальных текстур на PBR материалы.\nВключает освещение.", false)) {
        g_app.rtxSelectedIndex = 1;
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
}
