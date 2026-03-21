#include "UIManager.h"
#include "Theme.h"

#include <imgui.h>

// ─────────────────────────────────────────────────────────────────────────────
// render()  –  point d'entrée : fullscreen window + layout global
// ─────────────────────────────────────────────────────────────────────────────
void UIManager::render()
{
    if (!m_fetchTriggered) { m_fetcher.fetch(); m_fetchTriggered = true; }

    ImGuiIO& io = ImGui::GetIO();

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

    const float topH  = 48.f;
    const float sideW = 230.f;
    const float contX = sideW;
    const float contY = topH;
    const float contW = io.DisplaySize.x - sideW;
    const float contH = io.DisplaySize.y - topH;

    renderTopBar(topH);
    renderSidebar(topH, sideW, io.DisplaySize.y - topH);
    renderMainContent(contX, contY, contW, contH);

    ImGui::End();

    // ── Modales (hors de la fenêtre racine) ───────────────────────────────────
    m_installModal.render(m_fetcher, m_installedDirty);
    m_deleteModal.render(m_installedDirty);
    m_newProjectModal.render(m_projectsPath);
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

    ImGui::SetCursorPos({20.f, 13.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
    ImGui::Text("Blendgment");
    ImGui::PopStyleColor();

    ImGui::SameLine(0.f, 8.f);
    ImGui::SetCursorPosY(15.f);
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("v0.3.0");
    ImGui::PopStyleColor();

    float statusX = io.DisplaySize.x - 180.f;
    ImGui::SetCursorPos({statusX, 15.f});
    ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
    ImGui::Text("Pret");
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();

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
        case NavPage::Dashboard:
            m_dashPage.render(m_fetcher, m_installedVersions,
                              m_installedDirty, m_installPath, m_projectsPath, m_deleteModal);
            break;
        case NavPage::Versions:
            m_versionsPage.render(m_fetcher, m_installedVersions,
                                  m_installedDirty, m_installPath, m_installModal);
            break;
        case NavPage::Projects:
            m_projectsPage.render(m_projectsPath, m_newProjectModal, m_addAssetModal, m_installPath);
            break;
        case NavPage::Settings:
            m_settingsPage.render(m_installPath, m_projectsPath);
            break;
    }

    ImGui::EndChild();
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
