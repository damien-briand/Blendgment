#pragma once

#include <string>
#include <vector>
#include "../InstalledVersion.h"

// ─────────────────────────────────────────────────────────────────────────────
// Modale d'ajout d'un asset (sous-projet) à un grand projet
// ─────────────────────────────────────────────────────────────────────────────
class AddAssetModal {
public:
    /// Ouvre la modale pour ajouter un asset à un grand projet
    void open(const char* grandProjectPath, const char* installPath);

    /// Rendu ImGUI
    void render();

    bool isVisible() const { return m_visible; }

private:
    bool                          m_visible       = false;
    char                          m_name[256]     = {};
    std::string                   m_errorMsg;
    std::vector<InstalledVersion> m_versions;
    int                           m_selectedIdx   = -1;
    std::string                   m_grandProjectPath;
    const char*                   m_installPath   = nullptr;
};
