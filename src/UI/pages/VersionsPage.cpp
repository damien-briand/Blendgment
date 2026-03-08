#include "VersionsPage.h"
#include "../Theme.h"
#include "../InstalledVersion.h"
#include "../modals/InstallModal.h"

#include <imgui.h>
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
void VersionsPage::render(BlenderFetcher&                 fetcher,
                          std::vector<InstalledVersion>&  versions,
                          bool&                           installedDirty,
                          const char*                     installPath,
                          InstallModal&                   installModal)
{
    // Rescan si nécessaire (ex : installation terminée pendant qu'on est ici)
    if (installedDirty)
        scanInstalledVersions(versions, installedDirty, installPath);

    // Lambda : vérifie si une version major.minor est déjà installée
    auto isInstalled = [&](const std::string& mm) -> bool {
        std::string prefix = mm + ".";
        for (const auto& iv : versions)
            if (iv.version.rfind(prefix, 0) == 0) return true;
        return false;
    };
    // Lambda : retourne le patch installé pour un major.minor donné
    auto installedPatch = [&](const std::string& mm) -> std::string {
        std::string prefix = mm + ".";
        for (const auto& iv : versions)
            if (iv.version.rfind(prefix, 0) == 0) return iv.version;
        return "";
    };

    bool        loading  = fetcher.isLoading();
    bool        failed   = fetcher.hasFailed();
    bool        hasData  = fetcher.hasData();
    auto        vlist    = hasData ? fetcher.getVersions() : std::vector<BlenderVersion>{};

    // ── Titre ─────────────────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Versions Blender");
    ImGui::PopStyleColor();

    // ── Ligne de statut ───────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 58.f});
    if (loading) {
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text("Chargement depuis download.blender.org...");
        ImGui::PopStyleColor();
    } else if (failed) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
        ImGui::Text("Erreur de connexion.");
        ImGui::PopStyleColor();
    } else if (hasData) {
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("%zu versions trouvees  |  Derniere : Blender %s",
                    vlist.size(), fetcher.getLatestVersion().c_str());
        ImGui::PopStyleColor();
    }

    // ── Controles ─────────────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 90.f});

    if (loading) ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button(loading ? "Actualisation..." : "  Actualiser", ImVec2(150.f, 32.f)))
        fetcher.fetch();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    if (loading) ImGui::EndDisabled();

    ImGui::SameLine(0.f, 14.f);
    ImGui::SetCursorPosY(97.f);
    ImGui::Checkbox("Masquer versions < 2.80", &m_showRecentOnly);

    // ── Tableau des versions ──────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 140.f});
    float tableH = ImGui::GetContentRegionAvail().y - 16.f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg,         Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_TableRowBg,       ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,    ImVec4(0.16f, 0.16f, 0.21f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,    Col::BgCard);
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, Col::Separator);
    ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("##vtable", ImVec2(-16.f, tableH), false);

    if (!hasData && !failed) {
        ImGui::SetCursorPos({16.f, 16.f});
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text(loading ? "Chargement..." : "En attente...");
        ImGui::PopStyleColor();
    } else if (failed) {
        ImGui::SetCursorPos({16.f, 20.f});
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
        ImGui::Text("Impossible de contacter download.blender.org");
        ImGui::PopStyleColor();
        ImGui::SetCursorPos({16.f, 48.f});
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text("Verifiez votre connexion puis cliquez sur Actualiser.");
        ImGui::PopStyleColor();
    } else {
        if (ImGui::BeginTable("##vlist", 3,
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg,
            ImVec2(0.f, 0.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Version", ImGuiTableColumnFlags_WidthFixed,   160.f);
            ImGui::TableSetupColumn("Statut",  ImGuiTableColumnFlags_WidthFixed,   200.f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (int i = (int)vlist.size() - 1; i >= 0; --i) {
                const auto& v = vlist[i];

                if (m_showRecentOnly) {
                    int major = 0, minor = 0;
                    std::sscanf(v.version.c_str(), "%d.%d", &major, &minor);
                    if (major < 2 || (major == 2 && minor < 80)) continue;
                }

                ImGui::TableNextRow(0, 38.f);

                ImGui::TableSetColumnIndex(0);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
                ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
                ImGui::Text("Blender %s", v.version.c_str());
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(1);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
                std::string patch = installedPatch(v.version);
                if (!patch.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Col::Green);
                    ImGui::Text("  Installee (%s)", patch.c_str());
                    ImGui::PopStyleColor();
                } else if (v.isLatest) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Col::Green);
                    ImGui::Text("Derniere version");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
                    ImGui::Text("Disponible");
                    ImGui::PopStyleColor();
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);

                bool alreadyInstalled = !patch.empty();
                if (alreadyInstalled) {
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.38f, 0.22f, 0.55f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.38f, 0.22f, 0.55f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.38f, 0.22f, 0.55f));
                    ImGui::PushStyleColor(ImGuiCol_Text,          Col::Green);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
                    ImGui::BeginDisabled();
                    std::string badgeId = "  Installee##badge_" + v.version;
                    ImGui::Button(badgeId.c_str(), ImVec2(100.f, 26.f));
                    ImGui::EndDisabled();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(4);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button,
                        v.isLatest ? Col::Accent      : Col::BgCard);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        v.isLatest ? Col::AccentHover : ImVec4(0.25f, 0.25f, 0.32f, 1.f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        v.isLatest ? Col::AccentPress : Col::BgPanel);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
                    std::string btnId = "Installer##" + v.version;
                    if (ImGui::Button(btnId.c_str(), ImVec2(100.f, 26.f))) {
                        auto plat = SynchronousDownloader::currentPlatform();
                        installModal.open(v.version, installPath, plat.defaultFormat);
                        fetcher.fetchReleases(v.version);
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(3);
                }
            }
            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);
}
