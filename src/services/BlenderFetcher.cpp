#include "BlenderFetcher.h"
#include <curl/curl.h>
#include <regex>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdexcept>

static const char* RELEASE_URL = "https://download.blender.org/release/";

// ─────────────────────────────────────────────────────────────────────────────
// cURL write callback (string)
// ─────────────────────────────────────────────────────────────────────────────
std::size_t BlenderFetcher::writeCallback(void* ptr, std::size_t size,
                                          std::size_t nmemb, std::string* buf)
{
    buf->append(static_cast<const char*>(ptr), size * nmemb);
    return size * nmemb;
}

// ─────────────────────────────────────────────────────────────────────────────
// HTTP GET (retourne le body)
// ─────────────────────────────────────────────────────────────────────────────
std::string BlenderFetcher::httpGet(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        20L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,      "Blendgment/0.1");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("cURL: ") + curl_easy_strerror(res));
    return body;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tri sémantique (vecteur d'entiers)
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<int> splitVer(const std::string& v)
{
    std::vector<int> parts;
    std::stringstream ss(v);
    std::string tok;
    while (std::getline(ss, tok, '.'))
        parts.push_back(std::stoi(tok));
    return parts;
}

static bool verLessStr(const std::string& a, const std::string& b)
{
    auto va = splitVer(a);
    auto vb = splitVer(b);
    for (std::size_t i = 0; i < std::min(va.size(), vb.size()); ++i)
        if (va[i] != vb[i]) return va[i] < vb[i];
    return va.size() < vb.size();
}

static bool verLess(const BlenderVersion& a, const BlenderVersion& b)
{
    return verLessStr(a.version, b.version);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parse page principale  → liste de versions "X.Y"
// ─────────────────────────────────────────────────────────────────────────────
std::vector<BlenderVersion> BlenderFetcher::parseVersionsHtml(const std::string& html)
{
    std::vector<BlenderVersion> result;
    std::regex re(R"(href="Blender(\d+\.\d+)/")");
    auto beg = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = beg; it != end; ++it) {
        std::string ver = (*it)[1].str();
        bool dup = false;
        for (const auto& v : result) if (v.version == ver) { dup = true; break; }
        if (!dup) result.push_back({ ver, false });
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parse sous-page  → liste de releases "X.Y.Z"
// Ex : href="blender-5.0.1-linux-x64.tar.xz"  ──▶  "5.0.1"
// ─────────────────────────────────────────────────────────────────────────────
std::vector<BlenderRelease> BlenderFetcher::parseReleasesHtml(const std::string& html,
                                                               const std::string& majorMinor)
{
    std::vector<BlenderRelease> result;
    std::regex re(R"(href="blender-(\d+\.\d+\.\d+)-[^"]+")");
    auto beg = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = beg; it != end; ++it) {
        std::string ver = (*it)[1].str();
        bool dup = false;
        for (const auto& r : result) if (r.fullVersion == ver) { dup = true; break; }
        if (!dup) result.push_back({ ver, majorMinor, false });
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// fetch()  –  récupère la liste des versions majeures/mineures
// ─────────────────────────────────────────────────────────────────────────────
void BlenderFetcher::fetch()
{
    if (m_loading.load()) return;
    m_loading = true;
    m_failed  = false;

    m_future = std::async(std::launch::async, [this]()
    {
        try {
            auto html     = httpGet(RELEASE_URL);
            auto versions = parseVersionsHtml(html);
            std::sort(versions.begin(), versions.end(), verLess);
            if (!versions.empty()) versions.back().isLatest = true;

            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_versions = std::move(versions);
                m_hasData  = true;
            }
            std::cout << "[BlenderFetcher] " << m_versions.size()
                      << " versions. Derniere : " << m_versions.back().version << '\n';
        }
        catch (const std::exception& e) {
            std::cerr << "[BlenderFetcher] " << e.what() << '\n';
            m_failed = true;
        }
        m_loading = false;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// fetchReleases()  –  récupère les releases patch pour une version "X.Y"
// ─────────────────────────────────────────────────────────────────────────────
void BlenderFetcher::fetchReleases(const std::string& majorMinor)
{
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_releasesCache.find(majorMinor);
        if (it != m_releasesCache.end() && (it->second.loading || it->second.hasData))
            return;  // déjà en cours ou terminé
        m_releasesCache[majorMinor].loading = true;
        m_releasesCache[majorMinor].failed  = false;
    }

    m_releasesFutures[majorMinor] = std::async(std::launch::async,
        [this, majorMinor]()
    {
        try {
            std::string url = "https://download.blender.org/release/Blender"
                              + majorMinor + "/";
            auto html     = httpGet(url);
            auto releases = parseReleasesHtml(html, majorMinor);

            // Tri croissant par version patch
            std::sort(releases.begin(), releases.end(),
                [](const BlenderRelease& a, const BlenderRelease& b) {
                    return verLessStr(a.fullVersion, b.fullVersion);
                });
            if (!releases.empty()) releases.back().isLatestPatch = true;

            std::lock_guard<std::mutex> lk(m_mutex);
            auto& cache    = m_releasesCache[majorMinor];
            cache.releases = std::move(releases);
            cache.loading  = false;
            cache.hasData  = true;

            std::cout << "[BlenderFetcher] Blender " << majorMinor
                      << " : " << cache.releases.size() << " patches. Dernier : "
                      << cache.releases.back().fullVersion << '\n';
        }
        catch (const std::exception& e) {
            std::cerr << "[BlenderFetcher] fetchReleases(" << majorMinor
                      << "): " << e.what() << '\n';
            std::lock_guard<std::mutex> lk(m_mutex);
            auto& cache  = m_releasesCache[majorMinor];
            cache.loading = false;
            cache.failed  = true;
        }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// Getters thread-safe
// ─────────────────────────────────────────────────────────────────────────────
std::vector<BlenderVersion> BlenderFetcher::getVersions()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_versions;
}

std::string BlenderFetcher::getLatestVersion()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_versions.empty() ? "" : m_versions.back().version;
}

bool BlenderFetcher::isReleasesLoading(const std::string& majorMinor)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_releasesCache.find(majorMinor);
    return it != m_releasesCache.end() && it->second.loading;
}

bool BlenderFetcher::hasReleases(const std::string& majorMinor)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_releasesCache.find(majorMinor);
    return it != m_releasesCache.end() && it->second.hasData;
}

bool BlenderFetcher::releasesFailed(const std::string& majorMinor)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_releasesCache.find(majorMinor);
    return it != m_releasesCache.end() && it->second.failed;
}

std::vector<BlenderRelease> BlenderFetcher::getReleases(const std::string& majorMinor)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_releasesCache.find(majorMinor);
    if (it == m_releasesCache.end()) return {};
    return it->second.releases;
}
