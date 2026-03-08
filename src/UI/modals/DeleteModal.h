#pragma once

#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Modale de confirmation de suppression d'une version Blender installée
// ─────────────────────────────────────────────────────────────────────────────
class DeleteModal {
public:
    /// Ouvrir la modale pour la version identifiée par dirName / fullPath.
    void open(const std::string& dirName, const std::string& fullPath);

    /// Rendu ImGUI – passe installedDirty à true si la suppression est confirmée.
    void render(bool& installedDirty);

    bool isVisible() const { return m_visible; }

private:
    bool        m_visible  = false;
    std::string m_dirName;
    std::string m_fullPath;
};
