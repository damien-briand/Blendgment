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
    ImGui::Text("Bienvenue dans Blendgment – gestionnaire de versions et de projets Blender.");
    ImGui::PopStyleColor();

    // ── Cartes de statistiques ────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 108.f});
    statCard("##c1", "Versions installees", "0", "Aucune version",     Col::Accent);
    ImGui::SameLine(0.f, 14.f);
    statCard("##c2", "Projets",             "0", "Aucun projet",       Col::Blue);
    ImGui::SameLine(0.f, 14.f);
    statCard("##c3", "Derniere version",  "4.4", "Disponible en ligne", Col::Green);

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
    ImGui::SetCursorPos({28.f, 28.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
    ImGui::Text("Versions Blender");
    ImGui::PopStyleColor();

    // ── Bouton installer ──────────────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 72.f});
    ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
    ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
    if (ImGui::Button("+ Installer une version", ImVec2(200.f, 36.f))) {
        // TODO : ouvrir le navigateur de versions (étape 3)
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // ── Tableau vide (placeholder) ────────────────────────────────────────────
    ImGui::SetCursorPos({28.f, 130.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgPanel);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("##vtable", ImVec2(680.f, 320.f), false);

    // En-tête
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Col::BgCard);
    ImGui::BeginChild("##vhead", ImVec2(680.f, 36.f), false);
    float cols[] = {180.f, 120.f, 140.f, 120.f};
    const char* headers[] = {"Version", "Statut", "Taille", "Actions"};
    float cx = 16.f;
    for (int i = 0; i < 4; ++i) {
        ImGui::SetCursorPos({cx, 10.f});
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("%s", headers[i]);
        ImGui::PopStyleColor();
        cx += cols[i];
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Ligne vide
    ImGui::SetCursorPos({16.f, 70.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("Aucune version installee. Cliquez sur '+ Installer une version'.");
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
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
