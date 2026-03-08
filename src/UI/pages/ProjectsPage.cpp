#include "ProjectsPage.h"
#include "../Theme.h"
#include "../modals/NewProjectModal.h"

#include <imgui.h>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
void ProjectsPage::render(const char*      projectsPath,
                          NewProjectModal& newProjectModal)
{
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
        newProjectModal.open();
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
                 : std::min((int)projects.size() * 58.f + 16.f, 380.f);
    ImGui::BeginChild("##proj_panel", ImVec2(660.f, panelH), false);

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

            // ── Chemin + bouton Ouvrir ────────────────────────────────────────
            ImGui::SetCursorPosX(38.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            std::string dispPath = projPath.string();
            if (dispPath.size() > 52) dispPath = "..." + dispPath.substr(dispPath.size() - 49);
            ImGui::Text("%s", dispPath.c_str());
            ImGui::PopStyleColor();

            ImGui::SameLine(listW - 116.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string openId = "  Ouvrir##open_" + name;
            if (ImGui::Button(openId.c_str(), ImVec2(100.f, 24.f))) {
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
}
