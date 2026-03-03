#pragma once

#include <imgui.h>
#include <string>
#include "services/BlenderFetcher.h"

// ─────────────────────────────────────────────────────────────────────────────
// Pages de navigation
// ─────────────────────────────────────────────────────────────────────────────
enum class NavPage { Dashboard, Versions, Projects, Settings };

// ─────────────────────────────────────────────────────────────────────────────
// UIManager  –  construit toute l'interface ImGUI de l'application
// ─────────────────────────────────────────────────────────────────────────────
class UIManager {
public:
    UIManager()  = default;
    ~UIManager() = default;

    // Appeler entre ImGui::NewFrame() et ImGui::Render()
    void render();

private:
    // ── Blocs principaux ──────────────────────────────────────────────────────
    void renderTopBar(float barHeight);
    void renderSidebar(float topOffset, float sideWidth, float height);
    void renderMainContent(float x, float y, float w, float h);

    // ── Pages ─────────────────────────────────────────────────────────────────
    void pageDashboard();
    void pageVersions();
    void pageProjects();
    void pageSettings();

    // ── Widgets réutilisables ─────────────────────────────────────────────────
    void statCard(const char* id, const char* title,
                  const char* value, const char* sub, ImVec4 accent);
    bool navButton(const char* label, bool selected, float width);

    // ── État ────────────────────────────────────────────────────
    NavPage        m_currentPage    = NavPage::Dashboard;

    BlenderFetcher m_fetcher;
    bool           m_fetchTriggered = false;
    bool           m_showRecentOnly = true;   // masquer versions < 2.80

    // Settings state
    char m_installPath [256] = "/opt/blender";
    char m_projectsPath[256] = "";
};
