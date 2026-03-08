#pragma once

class NewProjectModal;

// ─────────────────────────────────────────────────────────────────────────────
// Page Projets
// ─────────────────────────────────────────────────────────────────────────────
class ProjectsPage {
public:
    void render(const char*      projectsPath,
                NewProjectModal& newProjectModal);
};
