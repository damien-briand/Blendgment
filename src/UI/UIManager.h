#pragma once

#include <imgui.h>
#include <string>
#include <mutex>
#include <future>
#include <memory>
#include "services/BlenderFetcher.h"
#include "utils/SynchronousDownloader.h"

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

    // ── Modale installation ───────────────────────────────────────────────────
    void renderInstallModal();

    // ── Widgets réutilisables ─────────────────────────────────────────────────
    void statCard(const char* id, const char* title,
                  const char* value, const char* sub, ImVec4 accent);
    bool navButton(const char* label, bool selected, float width);

    // ── État navigation ────────────────────────────────────────────────────────
    NavPage        m_currentPage    = NavPage::Dashboard;
    BlenderFetcher m_fetcher;
    bool           m_fetchTriggered = false;
    bool           m_showRecentOnly = true;

    // ── État installation ─────────────────────────────────────────────────────
    struct InstallState {
        bool            visible       = false;
        std::string     majorMinor;           // "5.0"
        std::string     selectedPatch;        // "5.0.1" – vide = auto (dernier)
        ArchiveFormat   selectedFmt   = ArchiveFormat::TarXZ;
        char            outputDir[512]= {};

        // Phase téléchargement
        bool            downloading   = false;
        std::string     downloadedPath;       // rempli quand done

        // Progression (écrite depuis thread download, lue depuis thread UI)
        std::mutex      progressMutex;
        DownloadProgress progress;

        std::unique_ptr<SynchronousDownloader> downloader;
        std::future<void>                      future;

        void open(const std::string& mm, const std::string& dir, ArchiveFormat fmt) {
            visible        = true;
            majorMinor     = mm;
            selectedPatch  = "";
            selectedFmt    = fmt;
            downloading    = false;
            downloadedPath.clear();
            progress       = {};
            downloader.reset();
            strncpy(outputDir, dir.c_str(), sizeof(outputDir) - 1);
            outputDir[sizeof(outputDir) - 1] = '\0';
        }
    } m_install;

    // ── Settings state ────────────────────────────────────────────────────────
    char m_installPath [256] = "/opt/blender";
    char m_projectsPath[256] = "";
};
