#pragma once

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <future>

// ─────────────────────────────────────────────────────────────────────────────
struct BlenderVersion {
    std::string version;    // ex : "4.4",  "5.0"
    bool        isLatest = false;
};

// Version complète avec patch : "5.0.1"  (issue d'une sous-page)
struct BlenderRelease {
    std::string fullVersion;   // "5.0.1"
    std::string majorMinor;    // "5.0"
    bool        isLatestPatch = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// BlenderFetcher
// ─────────────────────────────────────────────────────────────────────────────
class BlenderFetcher {
public:
    BlenderFetcher()  = default;
    ~BlenderFetcher() = default;

    // ── Liste des versions majeures/mineures (page principale) ────────────────
    void fetch();
    bool isLoading() const { return m_loading.load(); }
    bool hasFailed() const { return m_failed.load();  }
    bool hasData()   const { return m_hasData.load(); }
    std::vector<BlenderVersion> getVersions();
    std::string                 getLatestVersion();

    // ── Releases patch pour une version donnée (sous-page Blender5.0/) ────────
    void fetchReleases(const std::string& majorMinor);
    bool isReleasesLoading(const std::string& majorMinor);
    bool hasReleases      (const std::string& majorMinor);
    bool releasesFailed   (const std::string& majorMinor);
    std::vector<BlenderRelease> getReleases(const std::string& majorMinor);

private:
    static std::size_t          writeCallback(void* ptr, std::size_t size,
                                              std::size_t nmemb, std::string* buf);
    static std::string          httpGet(const std::string& url);
    static std::vector<BlenderVersion> parseVersionsHtml(const std::string& html);
    static std::vector<BlenderRelease> parseReleasesHtml(const std::string& html,
                                                         const std::string& majorMinor);

    // ── État page principale ─────────────────────────────────────────────────
    std::atomic<bool>            m_loading { false };
    std::atomic<bool>            m_failed  { false };
    std::atomic<bool>            m_hasData { false };
    mutable std::mutex           m_mutex;
    std::vector<BlenderVersion>  m_versions;
    std::future<void>            m_future;

    // ── Cache des releases par version ────────────────────────────────────
    struct ReleaseCache {
        bool loading = false;
        bool failed  = false;
        bool hasData = false;
        std::vector<BlenderRelease> releases;
    };
    std::map<std::string, ReleaseCache>      m_releasesCache;    // protégé par m_mutex
    std::map<std::string, std::future<void>> m_releasesFutures;  // thread principal seulement
};
