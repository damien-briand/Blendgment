#include "AddAssetModal.h"
#include "../Theme.h"
#include "../InstalledVersion.h"

#include <imgui.h>
#include <filesystem>
#include <cstring>
#include <fstream>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
void AddAssetModal::open(const char* grandProjectPath, const char* installPath)
{
    m_visible = true;
    m_grandProjectPath = grandProjectPath;
    m_installPath = installPath;
    memset(m_name, 0, sizeof(m_name));
    m_errorMsg.clear();
    m_selectedIdx = -1;
    m_versions.clear();
    
    // Scanne les versions disponibles
    bool dummy = false;
    scanInstalledVersions(m_versions, dummy, installPath);
    
    // Sélectionne la première version par défaut
    if (!m_versions.empty()) {
        m_selectedIdx = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AddAssetModal::render()
{
    if (!m_visible) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##add_asset");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##add_asset", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        ImGui::Text("Ajouter un asset");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Champ nom ──────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Nom de l'asset :");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.22f, 0.22f, 0.28f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
        bool pressedEnter = ImGui::InputText("##asset_name", m_name, sizeof(m_name),
                                             ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // Focus automatique à l'ouverture
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        // ── Sélection version Blender ───────────────────────────────────────────
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Version Blender :");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,        Col::BgCard);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.22f, 0.22f, 0.28f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.25f, 0.25f, 0.32f, 1.f));
        ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);

        if (m_versions.empty()) {
            ImGui::Text("Aucune version trouvee");
        } else {
            if (ImGui::BeginCombo("##asset_version_combo", 
                    m_selectedIdx >= 0 ? m_versions[m_selectedIdx].version.c_str() : "Selectionnez...")) {
                for (size_t i = 0; i < m_versions.size(); ++i) {
                    bool isSelected = (i == (size_t)m_selectedIdx);
                    if (ImGui::Selectable(m_versions[i].version.c_str(), isSelected)) {
                        m_selectedIdx = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

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
            // Vérifie qu'une version est sélectionnée
            if (m_selectedIdx < 0 || m_selectedIdx >= (int)m_versions.size()) {
                m_errorMsg = "Selectionnez une version de Blender.";
                return;
            }

            std::string assetName(m_name);
            for (char c : assetName) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' ||
                    c == '?' || c == '"'  || c == '<' || c == '>' || c == '|') {
                    m_errorMsg = "Nom invalide : caracteres interdits ( / \\ : * ? \" < > | )";
                    return;
                }
            }
            std::error_code ec;
            fs::path assetsDir = fs::path(m_grandProjectPath) / "Assets";
            fs::path assetDir = assetsDir / assetName;
            if (fs::exists(assetDir, ec)) {
                m_errorMsg = "Un asset avec ce nom existe deja.";
                return;
            }
            fs::create_directories(assetDir, ec);
            if (ec) {
                m_errorMsg = "Impossible de creer le dossier : " + ec.message();
                return;
            }

            // ── Dossier textures ──────────────────────────────────────────────
            fs::create_directories(assetDir / "textures", ec);

            // ── Lance Blender pour créer le fichier .blend ─────────────────────
            const auto& selectedVersion = m_versions[m_selectedIdx];
            fs::path blendFilePath = assetDir / (assetName + ".blend");
            
            fs::path execDir = fs::path(selectedVersion.executable).parent_path();
            fs::path appDir = execDir.parent_path().parent_path();
            fs::path scriptPath = appDir / "resources" / "create_blend.py";
            
            std::string cmd = "\"" + selectedVersion.executable + "\" -b -P \"" + 
                            scriptPath.string() + "\" -- \"" + blendFilePath.string() + "\" &";
            std::system(cmd.c_str());

            // ── Créer le fichier .blendgment ──────────────────────────────────
            fs::path configPath = assetDir / ".blendgment";
            std::ofstream configFile(configPath);
            if (configFile.is_open()) {
                configFile << "version=" << selectedVersion.version << "\n";
                configFile << "type=simple\n";
                configFile << "created=" << assetName << "\n";
                configFile.close();
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
        if (ImGui::Button("  Annuler##addassetcancel", ImVec2(btnW, 34.f))) {
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
