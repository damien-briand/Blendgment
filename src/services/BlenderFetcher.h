#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <future>

// ─────────────────────────────────────────────────────────────────────────────
struct BlenderVersion
{
    std::string version;
    bool isLatest = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// BlenderFetcher – récupère la liste des versions publiées sur
//                  https://download.blender.org/release/
// ─────────────────────────────────────────────────────────────────────────────
class BlenderFetcher
{
public:
    BlenderFetcher() = default;
    ~BlenderFetcher() = default;

    // Lance un fetch asynchrone (ne bloque pas le thread principal)
    void fetch();

    bool isLoading() const { return m_loading.load(); }
    bool hasFailed() const { return m_failed.load(); }
    bool hasData() const { return m_hasData.load(); }

    // Thread-safe – retournent une copie
    std::vector<BlenderVersion> getVersions();
    std::string getLatestVersion();

private:
    static std::size_t writeCallback(void *ptr, std::size_t size,
                                     std::size_t nmemb, std::string *buf);
    static std::string httpGet(const std::string &url);
    static std::vector<BlenderVersion> parseHtml(const std::string &html);

    std::atomic<bool> m_loading{false};
    std::atomic<bool> m_failed{false};
    std::atomic<bool> m_hasData{false};
    mutable std::mutex m_mutex;
    std::vector<BlenderVersion> m_versions;
    std::future<void> m_future;
};
