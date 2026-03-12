#include "InstalledVersion.h"

#include <filesystem>
#include <algorithm>
#include <cstdlib>

void scanInstalledVersions(std::vector<InstalledVersion>& versions,
                           bool& installedDirty,
                           const char* installPath)
{
    namespace fs = std::filesystem;
    versions.clear();
    installedDirty = false;

    std::error_code ec;
    fs::path base = fs::weakly_canonical(fs::absolute(installPath), ec);
    if (!fs::is_directory(base, ec)) return;

    for (auto& entry : fs::directory_iterator(base, ec)) {
        if (!entry.is_directory(ec)) continue;
        std::string name = entry.path().filename().string();
        // Correspond à "blender-X.Y.Z" ou "blender-X.Y.Z-platform-arch"
        if (name.rfind("blender-", 0) != 0) continue;

        // Extraire le numéro de version : après "blender-" jusqu'au "-" suivant
        std::string rest    = name.substr(8);
        std::string version = rest.substr(0, rest.find('-'));
        if (version.empty()) continue;

        // Chercher l'exécutable
        fs::path exePath = entry.path() / "blender";
#ifdef _WIN32
        exePath = entry.path() / "blender.exe";
#endif
        std::string exeStr = fs::exists(exePath, ec) ? exePath.string() : "";

        versions.push_back({name, version, entry.path().string(), exeStr});
    }

    // Tri décroissant (version la plus récente en premier)
    std::sort(versions.begin(), versions.end(),
        [](const InstalledVersion& a, const InstalledVersion& b) {
            return a.version > b.version;
        });
}

void launchBlender(const InstalledVersion& v)
{
    if (v.executable.empty()) return;
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + v.executable + "\"";
    std::system(cmd.c_str());
#else
    std::string cmd = "\"" + v.executable + "\" &";
    std::system(cmd.c_str());
#endif
}
