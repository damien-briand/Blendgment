#include "InstalledVersion.h"

#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// Helper : Convert version string to tuple of ints for comparison
static std::tuple<int, int, int> parseVersion(const std::string& versionStr)
{
    int major = 0, minor = 0, patch = 0;
    std::istringstream iss(versionStr);
    char dot;
    iss >> major >> dot >> minor >> dot >> patch;
    return std::make_tuple(major, minor, patch);
}

// Helper : Find blender executable in various possible locations
static std::string findBlenderExecutable(const fs::path& blenderDir)
{
    std::error_code ec;
    std::vector<fs::path> searchPaths;

#ifdef _WIN32
    // Windows: Common paths for blender executable
    searchPaths.push_back(blenderDir / "blender.exe");
    searchPaths.push_back(blenderDir / "bin" / "blender.exe");
#else
    // Linux/macOS: Possible paths
    searchPaths.push_back(blenderDir / "blender");
    searchPaths.push_back(blenderDir / "bin" / "blender");
#endif

    for (const auto& path : searchPaths) {
        if (fs::exists(path, ec)) {
            return fs::absolute(path, ec).string();
        }
    }

    return "";  // Not found
}

void scanInstalledVersions(std::vector<InstalledVersion>& versions,
                           bool& installedDirty,
                           const char* installPath)
{
    versions.clear();
    installedDirty = false;

    std::error_code ec;
    fs::path base = fs::weakly_canonical(fs::absolute(installPath), ec);
    
    if (!fs::is_directory(base, ec)) {
        std::cerr << "[scanInstalledVersions] Chemin invalide: " << installPath << "\n";
        return;
    }

    for (auto& entry : fs::directory_iterator(base, ec)) {
        if (!entry.is_directory(ec)) continue;
        
        std::string name = entry.path().filename().string();
        std::string version;
        
        // Look for "blender-X.Y.Z" format (with or without platform suffix)
        if (name.rfind("blender-", 0) == 0) {
            // Extract version: after "blender-" until next "-"
            std::string rest = name.substr(8);
            size_t dashPos = rest.find('-');
            version = (dashPos == std::string::npos) ? rest : rest.substr(0, dashPos);
        }
        // Also check for simple "blender" folder (for portable installations)
        else if (name == "blender" && fs::is_directory(entry.path(), ec)) {
            // Try to find version in the folder structure
            version = "unknown";
        }
        else {
            continue;  // Skip folders that don't match naming convention
        }
        
        if (version.empty()) continue;

        // Find the executable
        std::string exeStr = findBlenderExecutable(entry.path());

        if (!exeStr.empty()) {
            versions.push_back({name, version, entry.path().string(), exeStr});
            std::cout << "[scanInstalledVersions] Found Blender " << version 
                      << " at " << entry.path().string() << "\n";
        } else {
            std::cerr << "[scanInstalledVersions] No executable found in: " 
                      << entry.path().string() << "\n";
        }
    }

    // Sort by version descending (newest first)
    // For strict version comparison, parse as semantic versioning
    std::sort(versions.begin(), versions.end(),
        [](const InstalledVersion& a, const InstalledVersion& b) {
            // Try semantic version comparison first
            if (a.version != "unknown" && b.version != "unknown") {
                auto versionA = parseVersion(a.version);
                auto versionB = parseVersion(b.version);
                
                int aMajor = std::get<0>(versionA);
                int aMinor = std::get<1>(versionA);
                int aPatch = std::get<2>(versionA);
                
                int bMajor = std::get<0>(versionB);
                int bMinor = std::get<1>(versionB);
                int bPatch = std::get<2>(versionB);
                
                if (aMajor != bMajor) return aMajor > bMajor;
                if (aMinor != bMinor) return aMinor > bMinor;
                return aPatch > bPatch;
            }
            
            // Fallback to string comparison
            return a.version > b.version;
        });
}

void launchBlender(const InstalledVersion& v)
{
    if (v.executable.empty()) {
        std::cerr << "[launchBlender] No executable path provided\n";
        return;
    }

#ifdef _WIN32
    // Windows: Use CreateProcess or start command
    // The /B flag ensures it starts without a new window
    std::string cmd = "start \"\" \"" + v.executable + "\"";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "[launchBlender] Failed to launch on Windows: " << v.executable << "\n";
    } else {
        std::cout << "[launchBlender] Launched Blender: " << v.executable << "\n";
    }
#else
    // Linux/macOS: Run in background with &
    std::string cmd = "\"" + v.executable + "\" &";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "[launchBlender] Failed to launch: " << v.executable << "\n";
    } else {
        std::cout << "[launchBlender] Launched Blender: " << v.executable << "\n";
    }
#endif
}
