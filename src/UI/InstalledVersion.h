#pragma once

#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Version Blender détectée dans le répertoire d'installation
// ─────────────────────────────────────────────────────────────────────────────
struct InstalledVersion {
    std::string dirName;     // "blender-4.4.3-linux-x64"
    std::string version;     // "4.4.3"
    std::string fullPath;    // chemin absolu du répertoire
    std::string executable;  // chemin absolu de l'exécutable (vide si absent)
};

// ─────────────────────────────────────────────────────────────────────────────
// Utilitaires partagés sur les versions installées
// ─────────────────────────────────────────────────────────────────────────────

/// Scanne installPath, remplit versions et remet installedDirty à false.
void scanInstalledVersions(std::vector<InstalledVersion>& versions,
                           bool& installedDirty,
                           const char* installPath);

/// Lance Blender en arrière-plan (détache le processus).
void launchBlender(const InstalledVersion& v);
