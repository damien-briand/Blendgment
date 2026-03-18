#include "ProjectsPage.h"
#include "../Theme.h"
#include "../modals/NewProjectModal.h"
#include "../InstalledVersion.h"
#include "../ProjectMetadata.h"

#include <imgui.h>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdlib>
#include <system_error>

// ─────────────────────────────────────────────────────────────────────────────
void ProjectsPage::render(const char*      projectsPath,
                          NewProjectModal& newProjectModal,
                          const char*      installPath)
{
    m_installPath = installPath;
    namespace fs = std::filesystem;

    // ── Titre ─────────────────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Projets Blender");
    ImGui::PopStyleColor();

    // ── Bouton Nouveau projet ─────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 68.f});
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("+ Nouveau projet", ImVec2(160.f, 34.f)))
        newProjectModal.open(m_installPath);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // ── Scanner les projets existants ─────────────────────────────────────────
    std::vector<std::string> projects;
    std::error_code ec;
    fs::path base = fs::weakly_canonical(fs::absolute(projectsPath), ec);
    if (fs::is_directory(base, ec)) {
        for (auto& entry : fs::directory_iterator(base, ec))
            if (entry.is_directory(ec))
                projects.push_back(entry.path().filename().string());
        std::sort(projects.begin(), projects.end());
    }

    // ── Compteur ─────────────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 122.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
    ImGui::Text("%zu projet%s", projects.size(), projects.size() != 1 ? "s" : "");
    ImGui::PopStyleColor();

    // ── Liste des projets ─────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 146.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);

    float panelH = projects.empty() ? 72.f
                 : std::min((int)projects.size() * 58.f + 16.f, 700.f);
    ImGui::BeginChild("##proj_panel", ImVec2(900.f, panelH), false);

    if (projects.empty()) {
        ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(16.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text("Aucun projet. Cliquez sur '+ Nouveau projet' pour commencer.");
        ImGui::PopStyleColor();
    } else {
        float listW = ImGui::GetContentRegionAvail().x;
        ImGui::Spacing();

        for (size_t i = 0; i < projects.size(); ++i) {
            const auto& name = projects[i];
            fs::path projPath = base / name;

            // ── Lire la version du projet ──────────────────────────────────────
            ProjectMetadata metadata;
            std::string versionDisplay = "Sans version";
            if (ProjectMetadata::read(projPath.string(), metadata)) {
                versionDisplay = "v" + metadata.version;
            }

            // ── Icone + nom ──────────────────────────────────────────────────
            ImGui::SetCursorPosX(16.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
            ImGui::Text("[P]");
            ImGui::PopStyleColor();
            ImGui::SameLine(0.f, 10.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
            std::string dispName = name;
            if (dispName.size() > 44) dispName = dispName.substr(0, 41) + "...";
            ImGui::Text("%s", dispName.c_str());
            ImGui::PopStyleColor();
            
            ImGui::SameLine(0.f, 12.f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 1.f));
            ImGui::Text("[%s]", versionDisplay.c_str());
            ImGui::PopStyleColor();

            // ── Chemin + bouton Ouvrir ────────────────────────────────────────
            ImGui::SetCursorPosX(38.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            std::string dispPath = projPath.string();
            if (dispPath.size() > 52) dispPath = "..." + dispPath.substr(dispPath.size() - 49);
            ImGui::Text("%s", dispPath.c_str());
            ImGui::PopStyleColor();

            // ── Bouton Open In Blender ────────────────────────────────────────
            ImGui::SameLine(listW - 190.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string openBlId = "Blender##openbl_" + name;
            if (ImGui::Button(openBlId.c_str(), ImVec2(90.f, 24.f))) {
                m_blenderSelect.visible     = true;
                m_blenderSelect.projectPath = projPath.string();
                m_blenderSelect.projectName = name;
                m_blenderSelect.originalVersion = versionDisplay;
                m_blenderSelect.versions.clear();
                bool dummy = false;
                scanInstalledVersions(m_blenderSelect.versions, dummy, m_installPath);
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // ── Bouton Ouvrir ────────────────────────────────────────────────────
            ImGui::SameLine(listW - 285.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string openId = "Explore##open_" + name;
            if (ImGui::Button(openId.c_str(), ImVec2(85.f, 24.f))) {
#ifdef _WIN32
                std::string cmd = "explorer \"" + projPath.string() + "\"";
#elif __APPLE__
                std::string cmd = "open \"" + projPath.string() + "\"";
#else
                std::string cmd = "xdg-open \"" + projPath.string() + "\" &";
#endif
                std::system(cmd.c_str());
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // ── Bouton Supprimer ──────────────────────────────────────────────
            ImGui::SameLine(listW - 90.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.15f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.20f, 0.20f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.10f, 0.10f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string delId = "Suppr.##del_" + name;
            if (ImGui::Button(delId.c_str(), ImVec2(75.f, 24.f))) {
                m_deleteConfirm.visible  = true;
                m_deleteConfirm.name     = name;
                m_deleteConfirm.fullPath = projPath.string();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // ── Séparateur ───────────────────────────────────────────────────
            ImGui::Spacing();
            if (i + 1 < projects.size()) {
                ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
                ImGui::Separator();
                ImGui::PopStyleColor();
                ImGui::Spacing();
            }
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    renderDeleteConfirmModal();
    renderBlenderVersionModal();
}

// ─────────────────────────────────────────────────────────────────────────────
// Modale de confirmation de suppression d'un projet
// ─────────────────────────────────────────────────────────────────────────────
void ProjectsPage::renderDeleteConfirmModal()
{
    if (!m_deleteConfirm.visible) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##delete_project_confirm");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##delete_project_confirm", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.30f, 0.30f, 1.f));
        ImGui::Text("Supprimer le projet");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Message ────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::TextWrapped("Vous allez supprimer definitivement le dossier :");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::SetCursorPosX(12.f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgCard);
        ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##del_proj_path", ImVec2(ImGui::GetContentRegionAvail().x, 36.f), false);
        ImGui::SetCursorPos({10.f, 8.f});
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        std::string disp = m_deleteConfirm.name;
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

        // ── Boutons ────────────────────────────────────────────────────────────
        float btnW    = 120.f;
        float spacing = 12.f;
        float totalW  = btnW * 2.f + spacing;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalW) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.60f, 0.15f, 0.15f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.20f, 0.20f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.42f, 0.10f, 0.10f, 1.f));
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Supprimer", ImVec2(btnW, 34.f))) {
            std::error_code ec;
            std::filesystem::remove_all(m_deleteConfirm.fullPath, ec);
            m_deleteConfirm.visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0.f, spacing);

        ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Annuler##projdelcancel", ImVec2(btnW, 34.f))) {
            m_deleteConfirm.visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_deleteConfirm.visible = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Modale de sélection de version de Blender
// ─────────────────────────────────────────────────────────────────────────────
void ProjectsPage::renderBlenderVersionModal()
{
    if (!m_blenderSelect.visible) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##blender_version_select");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##blender_version_select", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
        ImGui::Text("Choisir une version de Blender");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Projet concerné ────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Projet :");
        ImGui::PopStyleColor();
        ImGui::SameLine(0.f, 12.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        ImGui::Text("%s", m_blenderSelect.projectName.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        // ── Version originale (recommandée) ────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
        ImGui::Text("Version recommandee :");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.16f, 0.12f, 1.f));  // Vert foncé
        ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##version_original", ImVec2(ImGui::GetContentRegionAvail().x, 50.f), false);
        ImGui::Spacing();
        ImGui::SetCursorPosX(16.f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 1.00f, 0.50f, 1.f));  // Vert clair
        ImGui::Text("%s (projet)", m_blenderSelect.originalVersion.c_str());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        // ── Autres versions disponibles ────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Autres versions :");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgCard);
        ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##version_list", ImVec2(ImGui::GetContentRegionAvail().x, 180.f), false);

        if (m_blenderSelect.versions.empty()) {
            ImGui::SetCursorPosY(80.f);
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 100.f) * 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            ImGui::Text("Aucune version installee");
            ImGui::PopStyleColor();
        } else {
            ImGui::Spacing();
            for (size_t i = 0; i < m_blenderSelect.versions.size(); ++i) {
                const auto& versionInfo = m_blenderSelect.versions[i];
                ImGui::SetCursorPosX(16.f);
                ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
                ImGui::Text("%s", versionInfo.version.c_str());
                ImGui::PopStyleColor();

                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 110.f);
                ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
                ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
                std::string btnId = "Ouvrir##bl_" + versionInfo.version;
                if (ImGui::Button(btnId.c_str(), ImVec2(90.f, 24.f))) {
                    std::string cmd = "\"" + versionInfo.executable + "\" \"" + m_blenderSelect.projectPath + "/" + 
                                      m_blenderSelect.projectName + ".blend\" &";
                    std::system(cmd.c_str());
                    m_blenderSelect.visible = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);

                ImGui::Spacing();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Bouton Annuler ────────────────────────────────────────────────────
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 104.f);
        ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Annuler##blvercancel", ImVec2(90.f, 34.f))) {
            m_blenderSelect.visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_blenderSelect.visible = false;
}
