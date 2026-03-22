#include "SettingsPage.h"
#include "../Theme.h"

#include <imgui.h>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::render(char* installPath, char* projectsPath,
                          std::function<void()> onSaveCallback,
                          std::function<void(char*)> onAutoConfigCallback)
{
    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Parametres");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({28.f, 80.f});

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.f);
    ImGui::BeginChild("##settings_panel", ImVec2(520.f, 280.f), false);

    auto labeledInput = [](const char* label, char* buf, size_t sz, float w) {
        ImGui::SetCursorPosX(20.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("%s", label);
        ImGui::PopStyleColor();
        ImGui::SetCursorPosX(20.f);
        ImGui::SetNextItemWidth(w);
        ImGui::InputText((std::string("##") + label).c_str(), buf, sz);
        ImGui::Spacing();
        ImGui::Spacing();
    };

    ImGui::SetCursorPosY(20.f);
    labeledInput("Repertoire d'installation Blender :", installPath,  256, 480.f);
    labeledInput("Repertoire des projets :",            projectsPath, 256, 480.f);

    // ── Bouton Auto-Config ────────────────────────────────────────────────────
    ImGui::SetCursorPosX(20.f);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.45f, 0.15f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.20f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.35f, 0.10f, 1.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("Auto-config", ImVec2(120.f, 34.f))) {
        if (onAutoConfigCallback) {
            onAutoConfigCallback(installPath);
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine(0.f, 12.f);
    
    // ── Bouton Sauvegarder ────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("Sauvegarder", ImVec2(120.f, 34.f))) {
        if (onSaveCallback) {
            onSaveCallback();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
