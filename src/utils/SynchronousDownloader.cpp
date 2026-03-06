#include "SynchronousDownloader.h"
#include <curl/curl.h>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// DownloadProgress::humanSize
// ─────────────────────────────────────────────────────────────────────────────
static std::string fmtBytes(int64_t b)
{
    char buf[64];
    if      (b >= 1024LL*1024*1024) snprintf(buf, sizeof(buf), "%.1f GB", b / (1024.0*1024*1024));
    else if (b >= 1024*1024)        snprintf(buf, sizeof(buf), "%.1f MB", b / (1024.0*1024));
    else if (b >= 1024)             snprintf(buf, sizeof(buf), "%.1f KB", b / 1024.0);
    else                            snprintf(buf, sizeof(buf), "%lld B",  (long long)b);
    return buf;
}

std::string DownloadProgress::humanSize() const
{
    if (total > 0) return fmtBytes(downloaded) + " / " + fmtBytes(total);
    return fmtBytes(downloaded);
}

// ─────────────────────────────────────────────────────────────────────────────
// Platform detection  (résolu à la compilation)
// ─────────────────────────────────────────────────────────────────────────────
DetectedPlatform SynchronousDownloader::currentPlatform()
{
    DetectedPlatform p;

#if defined(__linux__)
    p.os            = PlatOS::Linux;
    p.defaultFormat = ArchiveFormat::TarXZ;
#elif defined(__APPLE__)
    p.os            = PlatOS::macOS;
    p.defaultFormat = ArchiveFormat::DMG;
#elif defined(_WIN32)
    p.os            = PlatOS::Windows;
    p.defaultFormat = ArchiveFormat::Zip;
#else
    p.os            = PlatOS::Unknown;
    p.defaultFormat = ArchiveFormat::TarXZ;
#endif

#if defined(__x86_64__) || defined(_M_X64)
    p.arch = PlatArch::x64;
#elif defined(__aarch64__) || defined(_M_ARM64)
    p.arch = PlatArch::arm64;
#else
    p.arch = PlatArch::Unknown;
#endif

    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers: labels et extensions
// ─────────────────────────────────────────────────────────────────────────────
const char* SynchronousDownloader::formatExtension(ArchiveFormat fmt)
{
    switch (fmt) {
        case ArchiveFormat::TarXZ: return "tar.xz";
        case ArchiveFormat::DMG:   return "dmg";
        case ArchiveFormat::MSI:   return "msi";
        case ArchiveFormat::MSIX:  return "msix";
        case ArchiveFormat::Zip:   return "zip";
    }
    return "tar.xz";
}

const char* SynchronousDownloader::formatLabel(ArchiveFormat fmt)
{
    switch (fmt) {
        case ArchiveFormat::TarXZ: return "tar.xz  (Linux)";
        case ArchiveFormat::DMG:   return "DMG     (macOS)";
        case ArchiveFormat::MSI:   return "MSI     (Windows – installeur)";
        case ArchiveFormat::MSIX:  return "MSIX    (Windows – Store)";
        case ArchiveFormat::Zip:   return "ZIP     (Windows – portable)";
    }
    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// buildFilename  –  "blender-5.0.1-linux-x64.tar.xz"
// ─────────────────────────────────────────────────────────────────────────────
std::string SynchronousDownloader::buildFilename(const std::string& fullVersion,
                                                  PlatOS os, PlatArch arch,
                                                  ArchiveFormat fmt)
{
    const char* osStr = [os]() -> const char* {
        switch (os) {
            case PlatOS::Linux:   return "linux";
            case PlatOS::macOS:   return "macos";
            case PlatOS::Windows: return "windows";
            default:              return "linux";
        }
    }();

    const char* archStr = [arch]() -> const char* {
        switch (arch) {
            case PlatArch::x64:   return "x64";
            case PlatArch::arm64: return "arm64";
            default:              return "x64";
        }
    }();

    return "blender-" + fullVersion + "-" + osStr + "-" + archStr
           + "." + formatExtension(fmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildUrl
// ─────────────────────────────────────────────────────────────────────────────
std::string SynchronousDownloader::buildUrl(const std::string& majorMinor,
                                             const std::string& filename)
{
    return "https://download.blender.org/release/Blender"
           + majorMinor + "/" + filename;
}

// ─────────────────────────────────────────────────────────────────────────────
// cURL callbacks
// ─────────────────────────────────────────────────────────────────────────────
std::size_t SynchronousDownloader::writeCb(void* ptr, std::size_t size,
                                            std::size_t nmemb, void* ud)
{
    auto* ctx = static_cast<CurlCtx*>(ud);
    return fwrite(ptr, size, nmemb, ctx->fp);
}

int SynchronousDownloader::progressCb(void* ud, int64_t dltotal, int64_t dlnow,
                                       int64_t /*ultotal*/, int64_t /*ulnow*/)
{
    auto* ctx        = static_cast<CurlCtx*>(ud);
    ctx->prog->downloaded = dlnow;
    ctx->prog->total      = dltotal;

    if (ctx->cb && *ctx->cb && dlnow > 0)
        (*ctx->cb)(*ctx->prog);

    // Retourner != 0 annule le transfert
    return ctx->dl->m_cancelled ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// download()  –  bloquant, à appeler depuis un thread séparé
// ─────────────────────────────────────────────────────────────────────────────
std::string SynchronousDownloader::download(const std::string& url,
                                             const std::string& outputDir,
                                             ProgressCb onProgress)
{
    m_cancelled = false;

    // Créer le répertoire si nécessaire
    std::error_code ec;
    std::filesystem::create_directories(outputDir, ec);

    // Nom du fichier extrait depuis l'URL
    std::string filename = url.substr(url.rfind('/') + 1);
    std::string outPath  = outputDir + "/" + filename;

    FILE* fp = fopen(outPath.c_str(), "wb");
    if (!fp) {
        std::cerr << "[SynchronousDownloader] Impossible d'ouvrir : " << outPath << '\n';
        return "";
    }

    DownloadProgress prog;
    CurlCtx ctx { fp, this, &prog, &onProgress };

    CURL* curl = curl_easy_init();
    if (!curl) { fclose(fp); return ""; }

    curl_easy_setopt(curl, CURLOPT_URL,              url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,    writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,        &ctx);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCb);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA,     &ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS,       0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,   1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,          0L);      // pas de timeout global
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,   30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT,  100L);    // annuler si < 100 B/s
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,   30L);     //   pendant 30 s
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,   1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,        "Blendgment/0.1");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK || m_cancelled) {
        std::filesystem::remove(outPath, ec);  // supprimer le fichier partiel
        std::cerr << "[SynchronousDownloader] Erreur : "
                  << (m_cancelled ? "annule" : curl_easy_strerror(res)) << '\n';
        return "";
    }

    prog.done = true;
    if (onProgress) onProgress(prog);
    std::cout << "[SynchronousDownloader] Termine : " << outPath << '\n';
    return outPath;
}
