#include "DashboardPage.h"
#include "../Theme.h"
#include "../InstalledVersion.h"
#include "../modals/DeleteModal.h"

#include <imgui.h>
#include <algorithm>
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
void DashboardPage::render(BlenderFetcher&                 fetcher,
                           std::vector<InstalledVersion>&  versions,
                           bool&                           installedDirty,
                           const char*                     installPath,
                           DeleteModal&                    deleteModal)
{
    // ── Rescan si le répertoire a changé ou après une installation ────────────
    if (installedDirty)
        scanInstalledVersions(versions, installedDirty, installPath);

    int installedCount = (int)versions.size();

    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Dashboard");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({28.f, 54.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
    ImGui::Text("Bienvenue dans Blendgment, gestionnaire de versions et de projets Blender.");
    ImGui::PopStyleColor();

    // ── Cartes de statistiques ────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 108.f});
    {
        char countStr[8];
        snprintf(countStr, sizeof(countStr), "%d", installedCount);
        const char* sub = installedCount == 0 ? "Aucune version"
                        : installedCount == 1 ? "version installee"
                        :                       "versions installees";
        statCard("##c1", "Versions installees", countStr, sub, Col::Accent);
    }
    ImGui::SameLine(0.f, 14.f);
    statCard("##c2", "Projets", "0", "Aucun projet", Col::Blue);
    ImGui::SameLine(0.f, 14.f);
    {
        std::string latestVal = fetcher.hasData() ? fetcher.getLatestVersion() : "...";
        const char* latestSub = fetcher.isLoading() ? "Chargement..."
                              : fetcher.hasData()   ? "Disponible en ligne"
                              :                       "Aller sur Versions";
        statCard("##c3", "Derniere version", latestVal.c_str(), latestSub, Col::Green);
    }

    // ── Liste des versions installées ─────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 230.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
    ImGui::Text("Versions installees");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({28.f, 256.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);

    float panelH = installedCount == 0 ? 68.f
                 : std::min(installedCount * 66.f + 16.f, 320.f);
    ImGui::BeginChild("##installed_panel", ImVec2(620.f, panelH), false);

    if (versions.empty()) {
        ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(16.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text("Aucune version installee. Rendez-vous sur la page Versions pour en installer une.");
        ImGui::PopStyleColor();
    } else {
        float listW = ImGui::GetContentRegionAvail().x;
        ImGui::Spacing();

        for (size_t i = 0; i < versions.size(); ++i) {
            const auto& v = versions[i];
            bool canLaunch = !v.executable.empty();

            // ── Nom de version ───────────────────────────────────────────────
            ImGui::SetCursorPosX(16.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
            ImGui::Text("Blender %s", v.version.c_str());
            ImGui::PopStyleColor();

            // ── Répertoire + boutons sur la même ligne ───────────────────────
            ImGui::SetCursorPosX(16.f);
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            std::string displayDir = v.dirName;
            if (displayDir.size() > 42) displayDir = displayDir.substr(0, 39) + "...";
            ImGui::Text("%s", displayDir.c_str());
            ImGui::PopStyleColor();

            // Bouton Lancer
            ImGui::SameLine(listW - 228.f);
            if (!canLaunch) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string btnId = "  Lancer##lnch_" + v.dirName;
            if (ImGui::Button(btnId.c_str(), ImVec2(100.f, 26.f)))
                launchBlender(v);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            if (!canLaunch) ImGui::EndDisabled();

            // Bouton Supprimer
            ImGui::SameLine(listW - 116.f);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.15f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.20f, 0.20f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.10f, 0.10f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            std::string delId = "  Suppr.##del_" + v.dirName;
            if (ImGui::Button(delId.c_str(), ImVec2(100.f, 26.f)))
                deleteModal.open(v.dirName, v.fullPath);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // ── Séparateur ───────────────────────────────────────────────────
            ImGui::Spacing();
            if (i + 1 < versions.size()) {
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

// ─────────────────────────────────────────────────────────────────────────────
void DashboardPage::statCard(const char* id, const char* title,
                              const char* value, const char* sub, ImVec4 accent)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgCard);
    ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 10.f);
    ImGui::BeginChild(id, ImVec2(190.f, 96.f), false);

    ImGui::SetCursorPos({16.f, 14.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({16.f, 36.f});
    ImGui::PushStyleColor(ImGuiCol_Text, accent);
    ImGui::Text("%s", value);
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({16.f, 66.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("%s", sub);
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
