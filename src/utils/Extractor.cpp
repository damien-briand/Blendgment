#include "Extractor.h"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// Copie bloc par bloc d'une entrée vers le disque
// ─────────────────────────────────────────────────────────────────────────────
static int copyEntry(archive* src, archive* dst)
{
    for (;;) {
        const void* buf;
        std::size_t size;
        la_int64_t  offset;

        int r = archive_read_data_block(src, &buf, &size, &offset);
        if (r == ARCHIVE_EOF) return ARCHIVE_OK;
        if (r < ARCHIVE_OK)  return r;

        if (archive_write_data_block(dst, buf, size, offset) < ARCHIVE_OK)
            return ARCHIVE_FATAL;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// extract()
// ─────────────────────────────────────────────────────────────────────────────
std::string Extractor::extract(const std::string& archivePath,
                                const std::string& destDir,
                                ProgressCb         onProgress)
{
    m_cancelled = false;

    // Résoudre le chemin absolu pour éviter les ".." rejetés par NODOTDOT
    std::error_code ec;
    fs::path absDestDir = fs::weakly_canonical(fs::absolute(destDir), ec);
    if (ec) absDestDir = fs::absolute(destDir);
    std::string destAbs = absDestDir.string();

    // Créer le dossier de destination si nécessaire
    fs::create_directories(absDestDir, ec);

    // ── Ouvrir l'archive en lecture ──────────────────────────────────────────
    archive* src = archive_read_new();
    archive_read_support_filter_all(src);   // xz, gz, bz2, zstd…
    archive_read_support_format_all(src);   // tar, zip…

    if (archive_read_open_filename(src, archivePath.c_str(), 16384) != ARCHIVE_OK) {
        std::string err = archive_error_string(src);
        archive_read_free(src);
        std::cerr << "[Extractor] Impossible d'ouvrir : " << err << '\n';
        return "";
    }

    // ── Ouvrir le writer disque ───────────────────────────────────────────────
    archive* dst = archive_write_disk_new();
    archive_write_disk_set_options(dst,
        ARCHIVE_EXTRACT_TIME   |
        ARCHIVE_EXTRACT_PERM   |
        ARCHIVE_EXTRACT_SECURE_NODOTDOT);
    archive_write_disk_set_standard_lookup(dst);

    // Dossier de premier niveau (ex : "blender-5.0.1-linux-x64")
    std::string topDir;

    ExtractProgress prog;
    archive_entry* entry = nullptr;

    while (!m_cancelled) {
        int r = archive_read_next_header(src, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK) {
            std::cerr << "[Extractor] " << archive_error_string(src) << '\n';
            if (r < ARCHIVE_WARN) break;
        }

        // Préfixer le chemin avec destDir
        const char* origPath = archive_entry_pathname(entry);
        if (!origPath) continue;

        // Mémoriser le dossier racine de l'archive
        if (topDir.empty()) {
            std::string p = origPath;
            auto slash = p.find('/');
            topDir = (slash != std::string::npos) ? p.substr(0, slash) : p;
        }

        std::string fullPath = destAbs + "/" + origPath;
        archive_entry_set_pathname(entry, fullPath.c_str());

        // Écrire l'entrée
        if (archive_write_header(dst, entry) < ARCHIVE_OK) {
            std::cerr << "[Extractor] write_header: " << archive_error_string(dst) << '\n';
            continue;
        }

        if (archive_entry_size(entry) > 0) {
            if (copyEntry(src, dst) < ARCHIVE_WARN) break;
            prog.bytesWritten += static_cast<uint64_t>(archive_entry_size(entry));
        }

        prog.entriesDone++;
        prog.currentFile = origPath;

        // Callback toutes les 50 entrées pour ne pas saturer l'UI
        if (onProgress && (prog.entriesDone % 50 == 0))
            onProgress(prog);
    }

    archive_read_close(src);
    archive_read_free(src);
    archive_write_close(dst);
    archive_write_free(dst);

    if (m_cancelled) {
        std::cout << "[Extractor] Annule.\n";
        return "";
    }

    prog.done = true;
    if (onProgress) onProgress(prog);

    std::string extractedDir = destAbs + "/" + topDir;
    std::cout << "[Extractor] Termine : " << extractedDir
              << "  (" << prog.entriesDone << " entrees, "
              << prog.bytesWritten / (1024*1024) << " MB)\n";
    return extractedDir;
}
