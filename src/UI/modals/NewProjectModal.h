#pragma once

#include <string>
#include <vector>
#include "../InstalledVersion.h"

// ─────────────────────────────────────────────────────────────────────────────
// Modale de création d'un nouveau projet Blender
// ─────────────────────────────────────────────────────────────────────────────
class NewProjectModal {
public:
    /// Ouvre la modale et réinitialise les champs.
    void open(const char* installPath);

    /// Rendu ImGUI – crée le dossier dans projectsPath si confirmé.
    void render(const char* projectsPath);

    bool isVisible() const { return m_visible; }

private:
    bool                          m_visible       = false;
    char                          m_name[256]     = {};
    std::string                   m_errorMsg;
    std::vector<InstalledVersion> m_versions;
    int                           m_selectedIdx   = -1;
    const char*                   m_installPath   = nullptr;
};
