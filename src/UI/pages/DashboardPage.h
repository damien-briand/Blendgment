#pragma once

#include <vector>
#include <imgui.h>
#include "../InstalledVersion.h"
#include "services/BlenderFetcher.h"

class DeleteModal;

// ─────────────────────────────────────────────────────────────────────────────
// Page Dashboard
// ─────────────────────────────────────────────────────────────────────────────
class DashboardPage {
public:
    void render(BlenderFetcher&                  fetcher,
                std::vector<InstalledVersion>&   versions,
                bool&                            installedDirty,
                const char*                      installPath,
                const char*                      projectsPath,
                DeleteModal&                     deleteModal);

private:
    void statCard(const char* id, const char* title,
                  const char* value, const char* sub, ImVec4 accent);
};
