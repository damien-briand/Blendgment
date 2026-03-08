#include "InstallModal.h"
#include "../Theme.h"

#include <imgui.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
void InstallModal::open(const std::string& majorMinor,
                        const std::string& outputDir,
                        ArchiveFormat      fmt)
{
    m_visible        = true;
    m_majorMinor     = majorMinor;
    m_selectedPatch  = "";
    m_selectedFmt    = fmt;
    m_downloading    = false;
    m_extracting     = false;
    m_downloadedPath.clear();
    m_extractedDir.clear();
    m_progress        = {};
    m_extractProgress = {};
    m_downloader.reset();
    m_extractor.reset();
    strncpy(m_outputDir, outputDir.c_str(), sizeof(m_outputDir) - 1);
    m_outputDir[sizeof(m_outputDir) - 1] = '\0';
}

// ─────────────────────────────────────────────────────────────────────────────
void InstallModal::render(BlenderFetcher& fetcher, bool& installedDirty)
{
    if (!m_visible) return;

    // ── Vérifier si le futur (download+extract) est terminé ──────────────────
    if ((m_downloading || m_extracting) && m_future.valid()) {
        if (m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_future.get();
            m_downloading = false;
            m_extracting  = false;
        }
    }

    // ── Auto-sélection du patch le plus récent ────────────────────────────────
    if (m_selectedPatch.empty() && fetcher.hasReleases(m_majorMinor)) {
        auto releases = fetcher.getReleases(m_majorMinor);
        if (!releases.empty())
            m_selectedPatch = releases.back().fullVersion;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(560.f, 0.f), ImGuiCond_Always);
    ImGui::OpenPopup("##install_modal");

    ImGui::PushStyleColor(ImGuiCol_PopupBg,          Col::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding,  ImVec2(28.f, 24.f));
    ImGui::PushStyleVar  (ImGuiStyleVar_WindowRounding, 12.f);

    bool open = true;
    if (ImGui::BeginPopupModal("##install_modal", &open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // ── Titre ──────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
        ImGui::Text("Installer Blender %s", m_majorMinor.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, Col::Separator);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        auto plat = SynchronousDownloader::currentPlatform();

        // ── Plateforme détectée ────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        const char* osLabel = (plat.os == PlatOS::Linux)   ? "Linux"
                            : (plat.os == PlatOS::macOS)   ? "macOS"
                            : (plat.os == PlatOS::Windows) ? "Windows"
                            : "Inconnu";
        const char* archLabel = (plat.arch == PlatArch::x64)   ? "x64"
                              : (plat.arch == PlatArch::arm64) ? "arm64"
                              : "?";
        ImGui::Text("Plateforme detectee :"); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, Col::Accent);
        ImGui::Text("%s %s", osLabel, archLabel);
        ImGui::PopStyleColor(2);
        ImGui::Spacing();

        // ── Sélection de la version patch ──────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Version :");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(200.f);

        bool relLoading = fetcher.isReleasesLoading(m_majorMinor);
        bool relHas     = fetcher.hasReleases(m_majorMinor);
        bool relFailed  = fetcher.releasesFailed(m_majorMinor);

        if (relLoading) {
            ImGui::BeginDisabled();
            if (ImGui::BeginCombo("##patch", "Chargement..."))
                ImGui::EndCombo();
            ImGui::EndDisabled();
        } else if (relFailed) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
            ImGui::Text("Erreur de chargement des releases");
            ImGui::PopStyleColor();
        } else if (relHas) {
            auto releases = fetcher.getReleases(m_majorMinor);
            const char* preview = m_selectedPatch.empty()
                                ? "Selectionner..." : m_selectedPatch.c_str();
            if (ImGui::BeginCombo("##patch", preview)) {
                // Du plus récent au plus ancien
                for (int i = (int)releases.size() - 1; i >= 0; --i) {
                    const auto& r = releases[i];
                    std::string label = r.fullVersion;
                    if (r.isLatestPatch) label += "  (derniere)";
                    bool sel = (m_selectedPatch == r.fullVersion);
                    if (ImGui::Selectable(label.c_str(), sel))
                        m_selectedPatch = r.fullVersion;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Spacing();

        // ── Format d'archive ───────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Format :");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(280.f);
        if (plat.os == PlatOS::Windows) {
            static const ArchiveFormat winFmts[] = {
                ArchiveFormat::Zip, ArchiveFormat::MSI, ArchiveFormat::MSIX
            };
            if (ImGui::BeginCombo("##fmt",
                    SynchronousDownloader::formatLabel(m_selectedFmt)))
            {
                for (auto f : winFmts) {
                    bool sel = (m_selectedFmt == f);
                    if (ImGui::Selectable(SynchronousDownloader::formatLabel(f), sel))
                        m_selectedFmt = f;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Text);
            ImGui::Text("%s", SynchronousDownloader::formatLabel(plat.defaultFormat));
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // ── Répertoire de destination ──────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
        ImGui::Text("Dossier de destination :");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(504.f);
        ImGui::InputText("##outdir", m_outputDir, sizeof(m_outputDir));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ── Lire la progression sous mutex ────────────────────────────────────
        DownloadProgress  prog;
        ExtractProgress   extProg;
        {
            std::lock_guard<std::mutex> lk(m_progressMutex);
            prog    = m_progress;
            extProg = m_extractProgress;
        }

        if (m_downloading) {
            // ── Téléchargement en cours ────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
            ImGui::Text("Telechargement...");
            ImGui::PopStyleColor();

            char progLabel[64];
            snprintf(progLabel, sizeof(progLabel), "%.0f%%", prog.fraction() * 100.f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Col::Accent);
            ImGui::ProgressBar(prog.fraction(), ImVec2(-1.f, 18.f), progLabel);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            ImGui::Text("%s", prog.humanSize().c_str());
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.18f, 0.18f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.22f, 0.22f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.12f, 0.12f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler", ImVec2(120.f, 32.f)) && m_downloader)
                m_downloader->cancel();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (m_extracting) {
            // ── Extraction en cours ────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextDim);
            ImGui::Text("Extraction en cours...");
            ImGui::PopStyleColor();

            float t = static_cast<float>(ImGui::GetTime());
            float p = (std::sin(t * 2.5f) * 0.5f + 0.5f) * 0.85f + 0.05f;
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Col::Accent);
            ImGui::ProgressBar(p, ImVec2(-1.f, 18.f), "");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            if (!extProg.currentFile.empty()) {
                std::string fname = extProg.currentFile;
                if (fname.size() > 55) fname = "..." + fname.substr(fname.size() - 52);
                ImGui::Text("%llu entrees  |  %s",
                            (unsigned long long)extProg.entriesDone, fname.c_str());
            } else {
                ImGui::Text("Preparation...");
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.18f, 0.18f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.22f, 0.22f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.12f, 0.12f, 1.f));
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler##ext", ImVec2(120.f, 32.f)) && m_extractor)
                m_extractor->cancel();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (extProg.done) {
            // ── Succès ────────────────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, Col::Green);
            ImGui::Text("\u2713 Installation terminee !");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            ImGui::TextWrapped("%s", m_extractedDir.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Fermer", ImVec2(100.f, 32.f))) {
                m_visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else if (extProg.failed || prog.failed) {
            // ── Échec ──────────────────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
            ImGui::Text(extProg.failed ? "\u2715 Echec de l'extraction"
                                       : "\u2715 Echec du telechargement");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, Col::TextHint);
            const std::string& errMsg = extProg.failed ? extProg.error : prog.error;
            if (!errMsg.empty()) ImGui::TextWrapped("%s", errMsg.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Fermer", ImVec2(100.f, 32.f))) {
                m_visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

        } else {
            // ── Boutons Télécharger / Annuler ──────────────────────────────────
            bool canDownload = !m_selectedPatch.empty() && relHas;

            if (!canDownload) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button,        Col::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Col::AccentHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::AccentPress);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Telecharger", ImVec2(140.f, 32.f))) {
                std::string filename = SynchronousDownloader::buildFilename(
                    m_selectedPatch, plat.os, plat.arch, m_selectedFmt);
                std::string url    = SynchronousDownloader::buildUrl(m_majorMinor, filename);
                std::string outDir = m_outputDir;

                m_downloading     = true;
                m_progress        = {};
                m_extractProgress = {};
                m_downloader      = std::make_unique<SynchronousDownloader>();
                m_extractor       = std::make_unique<Extractor>();

                SynchronousDownloader* dlPtr  = m_downloader.get();
                Extractor*             extPtr = m_extractor.get();

                m_future = std::async(std::launch::async,
                    [this, dlPtr, extPtr, url, outDir]()
                    {
                        // ── 1. Téléchargement ─────────────────────────────────
                        auto dlCb = [this](const DownloadProgress& p) {
                            std::lock_guard<std::mutex> lk(m_progressMutex);
                            m_progress = p;
                        };
                        std::string archivePath = dlPtr->download(url, outDir, dlCb);

                        if (archivePath.empty()) {
                            if (!dlPtr->isCancelled()) {
                                std::lock_guard<std::mutex> lk(m_progressMutex);
                                m_progress.failed = true;
                                m_progress.error  = "Telechargement echoue.";
                            }
                            return;
                        }

                        // ── 2. Extraction ─────────────────────────────────────
                        {
                            std::lock_guard<std::mutex> lk(m_progressMutex);
                            m_downloading = false;
                            m_extracting  = true;
                        }

                        auto extCb = [this](const ExtractProgress& p) {
                            std::lock_guard<std::mutex> lk(m_progressMutex);
                            m_extractProgress = p;
                        };
                        std::string extracted = extPtr->extract(archivePath, outDir, extCb);

                        {
                            std::lock_guard<std::mutex> lk(m_progressMutex);
                            m_extracting = false;
                            if (!extracted.empty()) {
                                std::error_code ec;
                                std::filesystem::remove(archivePath, ec);
                                m_extractProgress.done = true;
                                m_extractedDir         = extracted;
                                // NOTE: installedDirty est mis à jour dans render()
                                // après vérification de done
                            } else if (!extPtr->isCancelled()) {
                                m_extractProgress.failed = true;
                                m_extractProgress.error  = "Extraction echouee.";
                            }
                        }
                    });
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            if (!canDownload) ImGui::EndDisabled();

            ImGui::SameLine(0.f, 12.f);

            ImGui::PushStyleColor(ImGuiCol_Button,        Col::BgCard);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.32f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Col::BgPanel);
            ImGui::PushStyleVar  (ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("Annuler##close", ImVec2(100.f, 32.f))) {
                m_visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        }

        // ── Propager installedDirty si extraction terminée ────────────────────
        {
            std::lock_guard<std::mutex> lk(m_progressMutex);
            if (m_extractProgress.done && !m_extractedDir.empty())
                installedDirty = true;
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    if (!open) m_visible = false;
}
