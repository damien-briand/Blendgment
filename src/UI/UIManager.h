#pragma once

#include <vector>
#include "InstalledVersion.h"
#include "services/BlenderFetcher.h"
#include "pages/DashboardPage.h"
#include "pages/VersionsPage.h"
#include "pages/ProjectsPage.h"
#include "pages/SettingsPage.h"
#include "modals/InstallModal.h"
#include "modals/DeleteModal.h"
#include "modals/NewProjectModal.h"

// ─────────────────────────────────────────────────────────────────────────────
// Pages de navigation
// ─────────────────────────────────────────────────────────────────────────────
enum class NavPage { Dashboard, Versions, Projects, Settings };

// ─────────────────────────────────────────────────────────────────────────────
// UIManager  –  orchestrateur principal de l'interface ImGUI
//               Gère le layout global et délègue à chaque page/modale.
// ─────────────────────────────────────────────────────────────────────────────
class UIManager {
public:
    UIManager()  = default;
    ~UIManager() = default;

    /// Appeler entre ImGui::NewFrame() et ImGui::Render()
    void render();

private:
    // ── Layout ────────────────────────────────────────────────────────────────
    void renderTopBar(float barHeight);
    void renderSidebar(float topOffset, float sideWidth, float height);
    void renderMainContent(float x, float y, float w, float h);

    // ── Widget navigation (sidebar) ───────────────────────────────────────────
    bool navButton(const char* label, bool selected, float width);

    // ── État partagé ─────────────────────────────────────────────────────────
    NavPage        m_currentPage    = NavPage::Dashboard;
    BlenderFetcher m_fetcher;
    bool           m_fetchTriggered = false;

    // Cache des versions installées (rescané quand dirty)
    std::vector<InstalledVersion> m_installedVersions;
    bool                          m_installedDirty = true;

    // Chemins configurables
    char m_installPath [256] = "../blender";
    char m_projectsPath[256] = "../projects";

    // ── Instances de pages ────────────────────────────────────────────────────
    DashboardPage  m_dashPage;
    VersionsPage   m_versionsPage;
    ProjectsPage   m_projectsPage;
    SettingsPage   m_settingsPage;

    // ── Instances de modales ──────────────────────────────────────────────────
    InstallModal      m_installModal;
    DeleteModal       m_deleteModal;
    NewProjectModal   m_newProjectModal;
};
