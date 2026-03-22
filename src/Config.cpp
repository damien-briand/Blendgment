#include "Config.h"
#include "UI/PlatformPaths.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

std::filesystem::path Config::getConfigPath()
{
    // Retourner le chemin dans le répertoire courant de Blendgment
    return fs::path("blendgment.cfg");
}

void Config::ensureConfigDir()
{
    // Le fichier est dans le répertoire courant,
    // pas besoin de créer des sous-répertoires
}


void Config::loadDefaults(std::string& installPath, std::string& projectsPath)
{
    // Use PlatformPaths to get sensible defaults
    auto defaultPaths = PlatformPaths::getDefaultBlenderPaths();
    
    if (!defaultPaths.empty()) {
        // Use the first existing path
        installPath = defaultPaths[0];
    } else {
        // Fallback to relative path
        #ifdef _WIN32
            installPath = ".\\blender";
        #else
            installPath = "./blender";
        #endif
    }
    
    // Get default projects path
    projectsPath = PlatformPaths::getDefaultProjectsPath();
    
    std::cout << "[Config] Loaded defaults:\n"
              << "  Install: " << installPath << "\n"
              << "  Projects: " << projectsPath << "\n";
}

void Config::load(std::string& installPath, std::string& projectsPath)
{
    fs::path configPath = getConfigPath();
    
    std::cout << "[Config] Loading from: " << configPath.string() << "\n";
    
    // Start with defaults
    loadDefaults(installPath, projectsPath);
    
    // Try to load from config file
    std::error_code ec;
    if (!fs::exists(configPath, ec)) {
        std::cout << "[Config] Config file doesn't exist yet, using defaults\n";
        return;
    }
    
    std::ifstream file(configPath.string());
    if (!file.is_open()) {
        std::cerr << "[Config] Failed to open config file\n";
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        
        // Parse key=value format
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        if (key == "blender_install") {
            installPath = value;
            std::cout << "[Config] Loaded blender_install: " << installPath << "\n";
        } else if (key == "projects_path") {
            projectsPath = value;
            std::cout << "[Config] Loaded projects_path: " << projectsPath << "\n";
        }
    }
    
    file.close();
}

void Config::save(const std::string& installPath, const std::string& projectsPath)
{
    ensureConfigDir();
    
    fs::path configPath = getConfigPath();
    
    std::cout << "[Config] Saving to: " << configPath.string() << "\n";
    
    std::ofstream file(configPath.string());
    if (!file.is_open()) {
        std::cerr << "[Config] Failed to open config file for writing\n";
        return;
    }
    
    file << "; Blendgment Configuration\n";
    file << "; This file is auto-generated. Do not edit manually.\n\n";
    file << "blender_install=" << installPath << "\n";
    file << "projects_path=" << projectsPath << "\n";
    
    file.close();
    
    std::cout << "[Config] Configuration saved successfully\n";
}
