#pragma once

#include <vector>
#include "../InstalledVersion.h"
#include "services/BlenderFetcher.h"

class InstallModal;

// ─────────────────────────────────────────────────────────────────────────────
// Page Versions Blender
// ─────────────────────────────────────────────────────────────────────────────
class VersionsPage {
public:
    void render(BlenderFetcher&                  fetcher,
                std::vector<InstalledVersion>&   versions,
                bool&                            installedDirty,
                const char*                      installPath,
                InstallModal&                    installModal);

private:
    bool m_showRecentOnly = true;
};
