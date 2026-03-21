#pragma once

#include <string>
#include <vector>
#include "../InstalledVersion.h"

class NewProjectModal;
class AddAssetModal;

// ─────────────────────────────────────────────────────────────────────────────
// Page Projets
// ─────────────────────────────────────────────────────────────────────────────
class ProjectsPage {
public:
    void render(const char*      projectsPath,
                NewProjectModal& newProjectModal,
                AddAssetModal&   addAssetModal,
                const char*      installPath);

private:
    void renderDeleteConfirmModal();
    void renderAssetDeleteConfirmModal();
    void renderBlenderVersionModal();

    struct DeleteConfirm {
        bool        visible  = false;
        std::string name;
        std::string fullPath;
    } m_deleteConfirm;

    struct AssetDeleteConfirm {
        bool        visible  = false;
        std::string assetName;
        std::string assetPath;
    } m_assetDeleteConfirm;

    struct BlenderVersionSelect {
        bool                              visible      = false;
        std::string                       projectPath;
        std::string                       projectName;
        std::string                       originalVersion;  // Version du projet
        std::vector<InstalledVersion>     versions;
    } m_blenderSelect;

    const char* m_installPath = nullptr;

    // ── Filtres ───────────────────────────────────────────────────────────────
    char        m_searchFilter[256]    = {};
    std::string m_versionFilter        = "";  // "" = tous
    bool        m_showSimple           = true;
    bool        m_showGrand            = true;
};
