#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Progression de l'extraction
// ─────────────────────────────────────────────────────────────────────────────
struct ExtractProgress {
    uint64_t    entriesDone  = 0;
    uint64_t    bytesWritten = 0;
    std::string currentFile;
    bool        done   = false;
    bool        failed = false;
    std::string error;
};

// ─────────────────────────────────────────────────────────────────────────────
// Extractor  –  extrait une archive (tar.xz, zip, tar.gz…) via libarchive.
//   Bloquant – à appeler depuis un thread séparé.
// ─────────────────────────────────────────────────────────────────────────────
class Extractor {
public:
    using ProgressCb = std::function<void(const ExtractProgress&)>;

    Extractor()  = default;
    ~Extractor() = default;

    // Retourne le chemin du dossier extrait, ou "" en cas d'erreur/annulation.
    // archivePath : chemin complet vers le fichier archive
    // destDir     : dossier dans lequel extraire
    std::string extract(const std::string& archivePath,
                        const std::string& destDir,
                        ProgressCb         onProgress = nullptr);

    void cancel()      { m_cancelled = true; }
    bool isCancelled() const { return m_cancelled.load(); }

private:
    std::atomic<bool> m_cancelled { false };
};
