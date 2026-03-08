#include "DeleteModal.h"
#include "../Theme.h"

#include <imgui.h>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
void DeleteModal::open(const std::string& dirName, const std::string& fullPath)
{
    m_visible  = true;
    m_dirName  = dirName;
    m_fullPath = fullPath;
}

// ─────────────────────────────────────────────────────────────────────────────
void DeleteModal::render(bool& installedDirty)
{
    if (!m_visible) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##delete_confirm");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##delete_confirm", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.30f, 0.30f, 1.f));
        ImGui::Text("Supprimer Blender");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Message ─────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::TextWrapped("Vous allez supprimer definitivement le dossier :");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::SetCursorPosX(12.f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgCard);
        ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##del_path", ImVec2(ImGui::GetContentRegionAvail().x, 36.f), false);
        ImGui::SetCursorPos({10.f, 8.f});
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        std::string disp = m_dirName;
        if (disp.size() > 52) disp = disp.substr(0, 49) + "...";
        ImGui::Text("%s", disp.c_str());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.f));
        ImGui::TextWrapped("Cette action est irreversible.");
        ImGui::PopStyleColor();

        ImGui::Spacing(); ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Boutons ─────────────────────────────────────────────────────────────
        float btnW    = 120.f;
        float spacing = 12.f;
        float totalW  = btnW * 2.f + spacing;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalW) * 0.5f);

        // Confirmer
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.60f, 0.15f, 0.15f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.20f, 0.20f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.42f, 0.10f, 0.10f, 1.f));
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Supprimer", ImVec2(btnW, 34.f))) {
            std::error_code ec;
            std::filesystem::remove_all(m_fullPath, ec);
            installedDirty = true;
            m_visible      = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0.f, spacing);

        // Annuler
        ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Annuler##delcancel", ImVec2(btnW, 34.f))) {
            m_visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_visible = false;
}
