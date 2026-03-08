#pragma once

#include <string>

class NewProjectModal;

// ─────────────────────────────────────────────────────────────────────────────
// Page Projets
// ─────────────────────────────────────────────────────────────────────────────
class ProjectsPage {
public:
    void render(const char*      projectsPath,
                NewProjectModal& newProjectModal);

private:
    void renderDeleteConfirmModal();

    struct DeleteConfirm {
        bool        visible  = false;
        std::string name;
        std::string fullPath;
    } m_deleteConfirm;
};
