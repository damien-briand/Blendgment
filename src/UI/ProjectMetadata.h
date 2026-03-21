#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// Métadonnées d'un projet Blendgment
// ─────────────────────────────────────────────────────────────────────────────
struct ProjectMetadata {
    std::string version;  // ex: "4.1.0"
    std::string type;     // "simple" ou "grand"
    
    /// Lit le fichier .blendgment du dossier projectPath
    /// Retourne true si le fichier existe et est valide
    static bool read(const std::string& projectPath, ProjectMetadata& out) {
        fs::path configPath = fs::path(projectPath) / ".blendgment";
        if (!fs::exists(configPath)) {
            return false;
        }

        std::ifstream file(configPath);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.find("version=") == 0) {
                out.version = line.substr(8);  // "version=" = 8 chars
            } else if (line.find("type=") == 0) {
                out.type = line.substr(5);     // "type=" = 5 chars
            }
        }
        file.close();
        return !out.version.empty();
    }
};
