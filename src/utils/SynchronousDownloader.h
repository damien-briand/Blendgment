#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Plateforme détectée à la compilation
// ─────────────────────────────────────────────────────────────────────────────
enum class PlatOS   { Linux, macOS, Windows, Unknown };
enum class PlatArch { x64, arm64, Unknown };

// Formats d'archive disponibles par OS
enum class ArchiveFormat {
    TarXZ,   // Linux   → .tar.xz  (seul format)
    DMG,     // macOS   → .dmg     (seul format)
    Zip,     // Windows → .zip     (recommandé)
    MSI,     // Windows → .msi     (installeur)
    MSIX,    // Windows → .msix    (Store)
};

struct DetectedPlatform {
    PlatOS        os            = PlatOS::Unknown;
    PlatArch      arch          = PlatArch::Unknown;
    ArchiveFormat defaultFormat = ArchiveFormat::TarXZ;
};

// ─────────────────────────────────────────────────────────────────────────────
// Progression du téléchargement
// ─────────────────────────────────────────────────────────────────────────────
struct DownloadProgress {
    int64_t     downloaded = 0;
    int64_t     total      = 0;
    bool        done       = false;
    bool        failed     = false;
    std::string error;

    float       fraction()  const { return (total > 0) ? float(downloaded) / float(total) : 0.f; }
    std::string humanSize() const; // "87.3 MB / 387.7 MB"
};

// ─────────────────────────────────────────────────────────────────────────────
// SynchronousDownloader  –  téléchargement bloquant via libcurl.
//   À appeler depuis un thread séparé pour ne pas figer l'UI.
// ─────────────────────────────────────────────────────────────────────────────
class SynchronousDownloader {
public:
    using ProgressCb = std::function<void(const DownloadProgress&)>;

    SynchronousDownloader()  = default;
    ~SynchronousDownloader() = default;

    // ── Détection de la plateforme ────────────────────────────────────────────
    static DetectedPlatform currentPlatform();

    // ── Constructeurs d'URL / noms de fichier ─────────────────────────────────
    // fullVersion : "5.0.1"  →  "blender-5.0.1-linux-x64.tar.xz"
    static std::string buildFilename(const std::string& fullVersion,
                                     PlatOS os, PlatArch arch,
                                     ArchiveFormat fmt);

    // majorMinor : "5.0"  →  "https://download.blender.org/release/Blender5.0/{filename}"
    static std::string buildUrl(const std::string& majorMinor,
                                const std::string& filename);

    // ── Helpers lisibles ─────────────────────────────────────────────────────
    static const char* formatLabel    (ArchiveFormat fmt);
    static const char* formatExtension(ArchiveFormat fmt);

    // ── Téléchargement bloquant ──────────────────────────────────────────────
    // Retourne le chemin du fichier sauvegardé, ou "" en cas d'erreur/annulation.
    std::string download(const std::string& url,
                         const std::string& outputDir,
                         ProgressCb         onProgress = nullptr);

    void cancel()      { m_cancelled = true; }
    bool isCancelled() const { return m_cancelled.load(); }

private:
    struct CurlCtx {
        FILE*                  fp;
        SynchronousDownloader* dl;
        DownloadProgress*      prog;
        ProgressCb*            cb;
    };

    static std::size_t writeCb    (void* ptr, std::size_t size, std::size_t nmemb, void* ud);
    static int         progressCb (void* ud, int64_t dltotal, int64_t dlnow,
                                   int64_t ultotal, int64_t ulnow);

    std::atomic<bool> m_cancelled { false };
};

