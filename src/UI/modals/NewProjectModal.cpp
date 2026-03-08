#include "NewProjectModal.h"
#include "../Theme.h"

#include <imgui.h>
#include <filesystem>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
void NewProjectModal::open()
{
    m_visible = true;
    memset(m_name, 0, sizeof(m_name));
    m_errorMsg.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
void NewProjectModal::render(const char* projectsPath)
{
    if (!m_visible) return;

    namespace fs = std::filesystem;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##new_project");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##new_project", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        ImGui::Text("Nouveau projet");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Champ nom ──────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Nom du projet :");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.22f, 0.22f, 0.28f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        bool pressedEnter = ImGui::InputText("##proj_name", m_name, sizeof(m_name),
                                             ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // Focus automatique à l'ouverture
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        // ── Message d'erreur ───────────────────────────────────────────────────
        if (!m_errorMsg.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.35f, 0.35f, 1.f));
            ImGui::TextWrapped("%s", m_errorMsg.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing(); ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Boutons ────────────────────────────────────────────────────────────
        float btnW   = 120.f;
        float totalW = btnW * 2.f + 12.f;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalW) * 0.5f);

        auto tryCreate = [&]() {
            std::string projName(m_name);
            for (char c : projName) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' ||
                    c == '?' || c == '"'  || c == '<' || c == '>' || c == '|') {
                    m_errorMsg = "Nom invalide : caracteres interdits ( / \\ : * ? \" < > | )";
                    return;
                }
            }
            std::error_code ec;
            fs::path base   = fs::weakly_canonical(fs::absolute(projectsPath), ec);
            fs::path newDir = base / projName;
            if (fs::exists(newDir, ec)) {
                m_errorMsg = "Un projet avec ce nom existe deja.";
                return;
            }
            fs::create_directories(newDir, ec);
            if (ec) {
                m_errorMsg = "Impossible de creer le dossier : " + ec.message();
                return;
            }

            // ── Dossier textures ──────────────────────────────────────────────
            fs::create_directories(newDir / "textures", ec);

            // ── Copie du fichier .blend de base ───────────────────────────────
            fs::path blendSrc = fs::weakly_canonical(
                fs::absolute("../BasicBlendFile.blend"), ec);
            if (fs::exists(blendSrc, ec)) {
                fs::path blendDst = newDir / (projName + ".blend");
                fs::copy_file(blendSrc, blendDst,
                              fs::copy_options::overwrite_existing, ec);
            }

            m_visible = false;
            ImGui::CloseCurrentPopup();
        };

        // Créer
        bool canCreate = m_name[0] != '\0';
        if (!canCreate) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Creer", ImVec2(btnW, 34.f)) || pressedEnter)
            tryCreate();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        if (!canCreate) ImGui::EndDisabled();

        ImGui::SameLine(0.f, 12.f);

        // Annuler
        ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("  Annuler##npcancel", ImVec2(btnW, 34.f))) {
            m_visible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_visible = false;
}
