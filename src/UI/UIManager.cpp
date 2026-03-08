#include "UIManager.h"
#include <imgui.h>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
// Palette de couleurs (thème sombre inspiré de Blender)
// ─────────────────────────────────────────────────────────────────────────────
namespace Col {
    static const ImVec4 BgDark      { 0.11f, 0.11f, 0.14f, 1.f };
    static const ImVec4 BgPanel     { 0.15f, 0.15f, 0.19f, 1.f };
    static const ImVec4 BgCard      { 0.18f, 0.18f, 0.23f, 1.f };
    static const ImVec4 BgSidebar   { 0.13f, 0.13f, 0.17f, 1.f };
    static const ImVec4 BgTopbar    { 0.10f, 0.10f, 0.13f, 1.f };
    static const ImVec4 Accent      { 0.87f, 0.45f, 0.10f, 1.f };  // orange Blender
    static const ImVec4 AccentHover { 0.96f, 0.56f, 0.16f, 1.f };
    static const ImVec4 AccentPress { 0.68f, 0.33f, 0.06f, 1.f };
    static const ImVec4 AccentDim   { 0.87f, 0.45f, 0.10f, 0.25f };
    static const ImVec4 Blue        { 0.28f, 0.65f, 0.90f, 1.f };
    static const ImVec4 Green       { 0.35f, 0.80f, 0.42f, 1.f };
    static const ImVec4 Text        { 0.92f, 0.92f, 0.92f, 1.f };
    static const ImVec4 TextDim     { 0.58f, 0.58f, 0.64f, 1.f };
    static const ImVec4 TextHint    { 0.42f, 0.42f, 0.48f, 1.f };
    static const ImVec4 Separator   { 0.28f, 0.28f, 0.35f, 1.f };
}

// ─────────────────────────────────────────────────────────────────────────────
// render()  –  point d'entrée : fullscreen window + layout global
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::render()
{
    if (!m_fetchTriggered) { m_fetcher.fetch(); m_fetchTriggered = true; }

    ImGuiIO& io = ImGui::GetIO();

    // Fenêtre racine sans décoration, couvre tout l'écran
    ImGuiWindowFlags rootFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse  |
        ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove      |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

    ImGui::SetNextWindowPos ({ 0.f, 0.f });
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("##root", nullptr, rootFlags);
    ImGui::PopStyleVar();

    const float topH    = 48.f;
    const float sideW   = 230.f;
    const float contX   = sideW;
    const float contY   = topH;
    const float contW   = io.DisplaySize.x - sideW;
    const float contH   = io.DisplaySize.y - topH;

    renderTopBar(topH);
    renderSidebar(topH, sideW, io.DisplaySize.y - topH);
    renderMainContent(contX, contY, contW, contH);

    ImGui::End();

    renderInstallModal();
    renderDeleteConfirmModal();
}

// ─────────────────────────────────────────────────────────────────────────────
// Top bar
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::renderTopBar(float barH)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetCursorPos({0.f, 0.f});

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgTopbar);
    ImGui::BeginChild("##topbar", ImVec2(io.DisplaySize.x, barH), false,
                      ImGuiWindowFlags_NoScrollbar);

    // ── Logo ──────────────────────────────────────────────────────────────────
    ImGui::SetCursorPos({20.f, 13.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
    ImGui::Text("Blendgment");
    ImGui::PopStyleColor();

    ImGui::SameLine(0.f, 8.f);
    ImGui::SetCursorPosY(15.f);
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("v0.1.0");
    ImGui::PopStyleColor();

    // ── Indicateur de statut (placeholder) ────────────────────────────────────
    float statusX = io.DisplaySize.x - 180.f;
    ImGui::SetCursorPos({statusX, 15.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("Prêt");
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Ligne séparatrice
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine({0.f, barH}, {io.DisplaySize.x, barH}, IM_COL32(50, 50, 65, 255), 1.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sidebar
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::renderSidebar(float topOffset, float sideW, float height)
{
    ImGui::SetCursorPos({0.f, topOffset});

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgSidebar);
    ImGui::BeginChild("##sidebar", ImVec2(sideW, height), false,
                      ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.f, 18.f));

    struct Item { const char* label; NavPage page; };
    static const Item items[] = {
        { "  Dashboard",  NavPage::Dashboard },
        { "  Versions",   NavPage::Versions  },
        { "  Projets",    NavPage::Projects  },
        { "  Parametres", NavPage::Settings  },
    };

    for (const auto& item : items) {
        bool sel = (m_currentPage == item.page);
        ImGui::SetCursorPosX(10.f);
        if (navButton(item.label, sel, sideW - 20.f))
            m_currentPage = item.page;

        ImGui::Dummy(ImVec2(0.f, 4.f));
    }

    // Séparateur bas
    ImDrawList* dl    = ImGui::GetWindowDrawList();
    ImVec2      wPos  = ImGui::GetWindowPos();
    ImVec2      wSize = ImGui::GetWindowSize();
    dl->AddLine(
        {wPos.x + wSize.x, wPos.y},
        {wPos.x + wSize.x, wPos.y + wSize.y},
        IM_COL32(50, 50, 65, 255), 1.f);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
// Zone de contenu principal
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::renderMainContent(float x, float y, float w, float h)
{
    ImGui::SetCursorPos({x, y});

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgDark);
    ImGui::BeginChild("##content", ImVec2(w, h), false);

    switch (m_currentPage) {
        case NavPage::Dashboard: pageDashboard(); break;
        case NavPage::Versions:  pageVersions();  break;
        case NavPage::Projects:  pageProjects();  break;
        case NavPage::Settings:  pageSettings();  break;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
// Pages
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::pageDashboard()
{
    // ── Rescan si le répertoire a changé ou après une installation ────────────
    if (m_installedDirty) scanInstalledVersions();

    int installedCount = (int)m_installedVersions.size();

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
        std::string latestVal = m_fetcher.hasData() ? m_fetcher.getLatestVersion() : "...";
        const char* latestSub = m_fetcher.isLoading() ? "Chargement..."
                              : m_fetcher.hasData()   ? "Disponible en ligne"
                              :                         "Aller sur Versions";
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

    if (m_installedVersions.empty()) {
        ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(16.f);
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
        ImGui::Text("Aucune version installee. Rendez-vous sur la page Versions pour en installer une.");
        ImGui::PopStyleColor();
    } else {
        float listW = ImGui::GetContentRegionAvail().x;
        ImGui::Spacing();

        for (size_t i = 0; i < m_installedVersions.size(); ++i) {
            const auto& v = m_installedVersions[i];
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
            if (ImGui::Button(delId.c_str(), ImVec2(100.f, 26.f))) {
                m_deleteConfirm.visible  = true;
                m_deleteConfirm.dirName  = v.dirName;
                m_deleteConfirm.fullPath = v.fullPath;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            // ── Séparateur ───────────────────────────────────────────────────
            ImGui::Spacing();
            if (i + 1 < m_installedVersions.size()) {
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
// Scan du répertoire d'installation pour détecter les versions installées
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::scanInstalledVersions()
{
    namespace fs = std::filesystem;
    m_installedVersions.clear();
    m_installedDirty = false;

    std::error_code ec;
    fs::path base = fs::weakly_canonical(fs::absolute(m_installPath), ec);
    if (!fs::is_directory(base, ec)) return;

    for (auto& entry : fs::directory_iterator(base, ec)) {
        if (!entry.is_directory(ec)) continue;
        std::string name = entry.path().filename().string();
        // Correspond à "blender-X.Y.Z" ou "blender-X.Y.Z-platform-arch"
        if (name.rfind("blender-", 0) != 0) continue;

        // Extraire le numéro de version : après "blender-" jusqu'au "-" suivant
        std::string rest    = name.substr(8);
        std::string version = rest.substr(0, rest.find('-'));
        if (version.empty()) continue;

        // Chercher l'exécutable
        fs::path exePath = entry.path() / "blender";
#ifdef _WIN32
        exePath = entry.path() / "blender.exe";
#endif
        std::string exeStr = fs::exists(exePath, ec) ? exePath.string() : "";

        m_installedVersions.push_back({name, version, entry.path().string(), exeStr});
    }

    // Tri décroissant (version la plus récente en premier)
    std::sort(m_installedVersions.begin(), m_installedVersions.end(),
        [](const InstalledVersion& a, const InstalledVersion& b) {
            return a.version > b.version;
        });
}

// ─────────────────────────────────────────────────────────────────────────────
// Lancement de Blender en arrière-plan
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::launchBlender(const InstalledVersion& v)
{
    if (v.executable.empty()) return;
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + v.executable + "\"";
    std::system(cmd.c_str());
#else
    std::string cmd = "\"" + v.executable + "\" &";
    std::system(cmd.c_str());
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
void UIManager::pageVersions()
{
    // Rescan si nécessaire (au cas où on arrive ici sans passer par le dashboard)
    if (m_installedDirty) scanInstalledVersions();

    // Lambda : vérifie si une version major.minor ("4.4") est déjà installée
    auto isInstalled = [&](const std::string& mm) -> bool {
        std::string prefix = mm + ".";
        for (const auto& iv : m_installedVersions)
            if (iv.version.rfind(prefix, 0) == 0) return true;
        return false;
    };
    // Lambda : retourne le patch installé pour un major.minor donné ("4.4.3")
    auto installedPatch = [&](const std::string& mm) -> std::string {
        std::string prefix = mm + ".";
        for (const auto& iv : m_installedVersions)
            if (iv.version.rfind(prefix, 0) == 0) return iv.version;
        return "";
    };

    bool        loading  = m_fetcher.isLoading();
    bool        failed   = m_fetcher.hasFailed();
    bool        hasData  = m_fetcher.hasData();
    auto        versions = hasData ? m_fetcher.getVersions() : std::vector<BlenderVersion>{};

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
                    versions.size(), m_fetcher.getLatestVersion().c_str());
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
        m_fetcher.fetch();
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

            // Du plus récent au plus ancien
            for (int i = (int)versions.size() - 1; i >= 0; --i) {
                const auto& v = versions[i];

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
                    // Version déjà installée : badge gris non cliquable
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
                        m_install.open(v.version, m_installPath, plat.defaultFormat);
                        m_fetcher.fetchReleases(v.version);
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

// ─────────────────────────────────────────────────────────────────────────────
void UIManager::pageProjects()
{
    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Projets Blender");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({28.f, 72.f});
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("+ Ajouter un projet", ImVec2(180.f, 36.f))) {
        // TODO : sélecteur de fichier .blend (étape 3)
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPos({28.f, 130.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("##ptable", ImVec2(680.f, 320.f), false);
    ImGui::SetCursorPos({16.f, 16.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("Aucun projet suivi. Cliquez sur '+ Ajouter un projet'.");
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
void UIManager::pageSettings()
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
    labeledInput("Repertoire d'installation Blender :", m_installPath,  sizeof(m_installPath),  480.f);
    labeledInput("Repertoire des projets :",            m_projectsPath, sizeof(m_projectsPath), 480.f);

    ImGui::SetCursorPosX(20.f);
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("Sauvegarder", ImVec2(120.f, 34.f))) {
        // TODO : persister la config (étape 3)
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
// Widget: carte de statistique
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::statCard(const char* id, const char* title,
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

// ─────────────────────────────────────────────────────────────────────────────
// Widget: bouton de navigation (sidebar)
// ─────────────────────────────────────────────────────────────────────────────
bool UIManager::navButton(const char* label, bool selected, float width)
{
    if (selected) {
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(Col::Accent.x * 0.25f, Col::Accent.y * 0.25f, Col::Accent.z * 0.25f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Text,   Col::TextDim);
    }
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.28f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.18f, 0.24f, 1.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_ButtonTextAlign, ImVec2(0.f, 0.5f));
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding,   7.f);

    bool clicked = ImGui::Button(label, ImVec2(width, 42.f));

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    return clicked;
}

// ─────────────────────────────────────────────────────────────────────────────
// Modale d'installation
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::renderInstallModal()
{
    if (!m_install.visible) return;

    // ── Vérifier si le futur (download+extract) est terminé ──────────────────
    if ((m_install.downloading || m_install.extracting) && m_install.future.valid()) {
        if (m_install.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_install.future.get();
            m_install.downloading = false;
            m_install.extracting  = false;
        }
    }

    // ── Auto-sélection du patch le plus récent ────────────────────────────────
    if (m_install.selectedPatch.empty() && m_fetcher.hasReleases(m_install.majorMinor)) {
        auto releases = m_fetcher.getReleases(m_install.majorMinor);
        if (!releases.empty())
            m_install.selectedPatch = releases.back().fullVersion;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(560.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##install_modal");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,    Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##install_modal", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        ImGui::Text("Installer Blender %s", m_install.majorMinor.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        auto plat = SynchronousDownloader::currentPlatform();

        // ── Plateforme détectée ────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        const char* osLabel = (plat.os == PlatOS::Linux)   ? "Linux"
                            : (plat.os == PlatOS::macOS)   ? "macOS"
                            : (plat.os == PlatOS::Windows) ? "Windows"
                            : "Inconnu";
        const char* archLabel = (plat.arch == PlatArch::x64)   ? "x64"
                              : (plat.arch == PlatArch::arm64) ? "arm64"
                              : "?";
        ImGui::Text("Plateforme detectee :"); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
        ImGui::Text("%s %s", osLabel, archLabel);
        ImGui::PopStyleColor(2);
        ImGui::Spacing();

        // ── Sélection de la version patch ──────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Version :");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(200.f);

        bool relLoading = m_fetcher.isReleasesLoading(m_install.majorMinor);
        bool relHas     = m_fetcher.hasReleases(m_install.majorMinor);
        bool relFailed  = m_fetcher.releasesFailed(m_install.majorMinor);

        if (relLoading) {
            ImGui::BeginDisabled();
            if (ImGui::BeginCombo("##patch", "Chargement..."))
                ImGui::EndCombo();
            ImGui::EndDisabled();
        } else if (relFailed) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
            ImGui::Text("Erreur de chargement des releases");
            ImGui::PopStyleColor();
        } else if (relHas) {
            auto releases = m_fetcher.getReleases(m_install.majorMinor);
            const char* preview = m_install.selectedPatch.empty()
                                ? "Selectionner..." : m_install.selectedPatch.c_str();
            if (ImGui::BeginCombo("##patch", preview)) {
                // Du plus récent au plus ancien
                for (int i = (int)releases.size() - 1; i >= 0; --i) {
                    const auto& r = releases[i];
                    std::string label = r.fullVersion;
                    if (r.isLatestPatch) label += "  (derniere)";
                    bool sel = (m_install.selectedPatch == r.fullVersion);
                    if (ImGui::Selectable(label.c_str(), sel))
                        m_install.selectedPatch = r.fullVersion;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Spacing();

        // ── Format d'archive ───────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Format :");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(280.f);
        if (plat.os == PlatOS::Windows) {
            // Sur Windows : choix entre zip, msi, msix
            static const ArchiveFormat winFmts[] = {
                ArchiveFormat::Zip, ArchiveFormat::MSI, ArchiveFormat::MSIX
            };
            if (ImGui::BeginCombo("##fmt",
                    SynchronousDownloader::formatLabel(m_install.selectedFmt)))
            {
                for (auto f : winFmts) {
                    bool sel = (m_install.selectedFmt == f);
                    if (ImGui::Selectable(SynchronousDownloader::formatLabel(f), sel))
                        m_install.selectedFmt = f;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        } else {
            // Linux / macOS : format unique, juste affiché
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
            ImGui::Text("%s", SynchronousDownloader::formatLabel(plat.defaultFormat));
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // ── Répertoire de destination ──────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Dossier de destination :");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(504.f);
        ImGui::InputText("##outdir", m_install.outputDir, sizeof(m_install.outputDir));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ── Zone progression / état ────────────────────────────────────────────
        DownloadProgress  prog;
        ExtractProgress   extProg;
        {
            std::lock_guard<std::mutex> lk(m_install.progressMutex);
            prog    = m_install.progress;
            extProg = m_install.extractProgress;
        }

        if (m_install.downloading) {
            // ── Téléchargement en cours ────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
            ImGui::Text("Telechargement...");
            ImGui::PopStyleColor();

            char progLabel[64];
            snprintf(progLabel, sizeof(progLabel), "%.0f%%", prog.fraction() * 100.f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Col::Accent);
            ImGui::ProgressBar(prog.fraction(), ImVec2(-1.f, 18.f), progLabel);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            ImGui::Text("%s", prog.humanSize().c_str());
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.18f, 0.18f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.22f, 0.22f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.12f, 0.12f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler", ImVec2(120.f, 32.f)) && m_install.downloader)
                m_install.downloader->cancel();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (m_install.extracting) {
            // ── Extraction en cours ────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
            ImGui::Text("Extraction en cours...");
            ImGui::PopStyleColor();

            // Barre indéterminée (on ne connaît pas le nombre total d'entrées)
            float t = static_cast<float>(ImGui::GetTime());
            float p = (std::sin(t * 2.5f) * 0.5f + 0.5f) * 0.85f + 0.05f;
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Col::Accent);
            ImGui::ProgressBar(p, ImVec2(-1.f, 18.f), "");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            if (!extProg.currentFile.empty()) {
                // Tronquer si trop long
                std::string fname = extProg.currentFile;
                if (fname.size() > 55) fname = "..." + fname.substr(fname.size() - 52);
                ImGui::Text("%llu entrees  |  %s",
                            (unsigned long long)extProg.entriesDone, fname.c_str());
            } else {
                ImGui::Text("Preparation...");
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.18f, 0.18f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.22f, 0.22f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.12f, 0.12f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler##ext", ImVec2(120.f, 32.f)) && m_install.extractor)
                m_install.extractor->cancel();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (extProg.done) {
            // ── Succès final ───────────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Green);
            ImGui::Text("\u2713 Installation terminee !");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            ImGui::TextWrapped("%s", m_install.extractedDir.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Fermer", ImVec2(100.f, 32.f))) {
                m_install.visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (extProg.failed || prog.failed) {
            // ── Échec ──────────────────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
            ImGui::Text(extProg.failed ? "\u2715 Echec de l'extraction"
                                       : "\u2715 Echec du telechargement");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            const std::string& errMsg = extProg.failed ? extProg.error : prog.error;
            if (!errMsg.empty()) ImGui::TextWrapped("%s", errMsg.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Fermer", ImVec2(100.f, 32.f))) {
                m_install.visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else {
            // ── Boutons Télécharger / Annuler ──────────────────────────────────
            bool canDownload = !m_install.selectedPatch.empty() && relHas;

            if (!canDownload) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Telecharger", ImVec2(140.f, 32.f))) {
                std::string filename = SynchronousDownloader::buildFilename(
                    m_install.selectedPatch, plat.os, plat.arch, m_install.selectedFmt);
                std::string url    = SynchronousDownloader::buildUrl(
                    m_install.majorMinor, filename);
                std::string outDir = m_install.outputDir;

                m_install.downloading   = true;
                m_install.progress      = {};
                m_install.extractProgress = {};
                m_install.downloader    = std::make_unique<SynchronousDownloader>();
                m_install.extractor     = std::make_unique<Extractor>();

                SynchronousDownloader* dlPtr  = m_install.downloader.get();
                Extractor*             extPtr = m_install.extractor.get();

                m_install.future = std::async(std::launch::async,
                    [this, dlPtr, extPtr, url, outDir]()
                    {
                        // ── 1. Téléchargement ─────────────────────────────────
                        auto dlCb = [this](const DownloadProgress& p) {
                            std::lock_guard<std::mutex> lk(m_install.progressMutex);
                            m_install.progress = p;
                        };
                        std::string archivePath = dlPtr->download(url, outDir, dlCb);

                        if (archivePath.empty()) {
                            if (!dlPtr->isCancelled()) {
                                std::lock_guard<std::mutex> lk(m_install.progressMutex);
                                m_install.progress.failed = true;
                                m_install.progress.error  = "Telechargement echoue.";
                            }
                            return;
                        }

                        // ── 2. Extraction ─────────────────────────────────────
                        {
                            std::lock_guard<std::mutex> lk(m_install.progressMutex);
                            m_install.downloading = false;
                            m_install.extracting  = true;
                        }

                        auto extCb = [this](const ExtractProgress& p) {
                            std::lock_guard<std::mutex> lk(m_install.progressMutex);
                            m_install.extractProgress = p;
                        };
                        std::string extracted = extPtr->extract(archivePath, outDir, extCb);

                        {
                            std::lock_guard<std::mutex> lk(m_install.progressMutex);
                            m_install.extracting = false;
                            if (!extracted.empty()) {
                                // Supprimer l'archive maintenant qu'elle n'est plus utile
                                std::error_code ec;
                                std::filesystem::remove(archivePath, ec);

                                m_install.extractProgress.done = true;
                                m_install.extractedDir         = extracted;
                                m_installedDirty               = true; // forcer un rescan du dashboard
                            } else if (!extPtr->isCancelled()) {
                                m_install.extractProgress.failed = true;
                                m_install.extractProgress.error  = "Extraction echouee.";
                            }
                        }
                    });
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            if (!canDownload) ImGui::EndDisabled();

            ImGui::SameLine(0.f, 12.f);

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler##close", ImVec2(100.f, 32.f))) {
                m_install.visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_install.visible = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Modale de confirmation de suppression
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::renderDeleteConfirmModal()
{
    if (!m_deleteConfirm.visible) return;

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
        std::string disp = m_deleteConfirm.dirName;
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
        float btnW = 120.f;
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
            std::filesystem::remove_all(m_deleteConfirm.fullPath, ec);
            m_installedDirty    = true;
            m_deleteConfirm.visible = false;
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
