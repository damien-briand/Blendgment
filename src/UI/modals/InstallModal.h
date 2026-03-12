#pragma once

#include <string>
#include <mutex>
#include <future>
#include <memory>
#include "services/BlenderFetcher.h"
#include "utils/SynchronousDownloader.h"
#include "utils/Extractor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Modale d'installation d'une version Blender
// (téléchargement + extraction avec suivi de progression)
// ─────────────────────────────────────────────────────────────────────────────
class InstallModal {
public:
    // ── Ouvrir la modale pour une version "major.minor" ───────────────────────
    void open(const std::string& majorMinor,
              const std::string& outputDir,
              ArchiveFormat      fmt);

    // ── Rendu ImGUI ───────────────────────────────────────────────────────────
    void render(BlenderFetcher& fetcher, bool& installedDirty);

    bool isVisible() const { return m_visible; }

private:
    // ── État ──────────────────────────────────────────────────────────────────
    bool          m_visible       = false;
    std::string   m_majorMinor;
    std::string   m_selectedPatch;
    ArchiveFormat m_selectedFmt   = ArchiveFormat::TarXZ;
    char          m_outputDir[512]= {};

    bool          m_downloading   = false;
    std::string   m_downloadedPath;

    bool          m_extracting    = false;
    std::string   m_extractedDir;

    std::unique_ptr<Extractor>              m_extractor;
    std::unique_ptr<SynchronousDownloader>  m_downloader;
    std::future<void>                       m_future;

    // Partagé UI-thread / worker-thread
    std::mutex       m_progressMutex;
    DownloadProgress m_progress;
    ExtractProgress  m_extractProgress;
};
