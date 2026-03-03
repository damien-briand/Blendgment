#include "UIManager.h"
#include <imgui.h>
#include <cstdio>

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
    statCard("##c1", "Versions installees", "0", "Aucune version",     Col::Accent);
    ImGui::SameLine(0.f, 14.f);
    statCard("##c2", "Projets",             "0", "Aucun projet",       Col::Blue);
    ImGui::SameLine(0.f, 14.f);
    {
        std::string latestVal = m_fetcher.hasData() ? m_fetcher.getLatestVersion() : "...";
        const char* latestSub = m_fetcher.isLoading() ? "Chargement..."
                              : m_fetcher.hasData()   ? "Disponible en ligne"
                              :                         "Aller sur Versions";
        statCard("##c3", "Derniere version", latestVal.c_str(), latestSub, Col::Green);
    }

    // ── Activité récente ──────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 230.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
    ImGui::Text("Activite recente");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({28.f, 256.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("##activity", ImVec2(560.f, 100.f), false);
    ImGui::SetCursorPos({16.f, 32.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("Aucune activite pour le moment.");
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
void UIManager::pageVersions()
{
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
                if (v.isLatest) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Col::Green);
                    ImGui::Text("\u25cf Derniere version");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
                    ImGui::Text("Disponible");
                    ImGui::PopStyleColor();
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
                ImGui::PushStyleColor(ImGuiCol_Button,
                    v.isLatest ? Col::Accent      : Col::BgCard);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                    v.isLatest ? Col::AccentHover : ImVec4(0.25f, 0.25f, 0.32f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                    v.isLatest ? Col::AccentPress : Col::BgPanel);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
                std::string btnId = "Installer##" + v.version;
                if (ImGui::Button(btnId.c_str(), ImVec2(100.f, 26.f))) {
                    // TODO : telechargement (etape 3)
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);
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
