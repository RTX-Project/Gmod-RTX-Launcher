#include "UI_Wizard.h"
#include "Globals.h"
#include "imgui.h"
extern void RenderSingleWizardStep(WizardStep currentStepToDraw, float childW);

void RenderUI_Wizard() {

        if (g_wizardIsSliding) {
            g_wizardSlideProgress += ImGui::GetIO().DeltaTime / 0.20f;
            if (g_wizardSlideProgress >= 1.0f) {
                g_wizardSlideProgress = 1.0f;
                g_wizardStep = g_wizardTargetStep;
                g_wizardIsSliding = false;
            }
        }

        ImGui::SetCursorPos(ImVec2(45, 42));
        ImGui::PushFont(g_imFontTitle);
        ImGui::Text(u8"МАСТЕР УСТАНОВКИ METROSTROI RTX");
        ImGui::PopFont();



        float childW = ImGui::GetWindowWidth() - 90.0f;
        ImGui::SetCursorPos(ImVec2(45, 115));
        ImGui::BeginChild("InstallerWizardChild", ImVec2(childW, 350), true);

        if (!g_wizardIsSliding) {
            RenderSingleWizardStep(g_wizardStep, childW);
        } else {
            float p = g_wizardSlideProgress;
            // Ease-out cubic
            float easeP = 1.0f - powf(1.0f - p, 3.0f);
            
            float slideDist = 350.0f; // Height of the child window
            float oldOffsetY = -slideDist * easeP;
            float newOffsetY = slideDist * (1.0f - easeP);

            // Draw Old Step
            ImGui::SetCursorPos(ImVec2(15.0f, 15.0f + oldOffsetY));
            ImGui::PushID(1);
            RenderSingleWizardStep(g_wizardStep, childW);
            ImGui::PopID();

            // Draw New Step
            ImGui::SetCursorPos(ImVec2(15.0f, 15.0f + newOffsetY));
            ImGui::PushID(2);
            RenderSingleWizardStep(g_wizardTargetStep, childW);
            ImGui::PopID();
        }

        ImGui::EndChild();
    }
