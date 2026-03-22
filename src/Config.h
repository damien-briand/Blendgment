#pragma once

#include <string>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
// Configuration persistante pour Blendgment
// Gère la sauvegarde et le chargement des paramètres utilisateur
// ─────────────────────────────────────────────────────────────────────────────
class Config {
public:
    /// Charge la configuration depuis le fichier (ou utilise les defaults)
    static void load(std::string& installPath, std::string& projectsPath);
    
    /// Sauvegarde la configuration dans le fichier
    static void save(const std::string& installPath, const std::string& projectsPath);
    
    /// Retourne le chemin du fichier de configuration
    static std::filesystem::path getConfigPath();

private:
    /// Crée le répertoire de config s'il n'existe pas
    static void ensureConfigDir();
    
    /// Charge les chemins par défaut selon la plateforme
    static void loadDefaults(std::string& installPath, std::string& projectsPath);
};
