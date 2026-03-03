#include "BlenderFetcher.h"
#include <curl/curl.h>
#include <regex>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdexcept>

static const char* RELEASE_URL = "https://download.blender.org/release/";

// ─────────────────────────────────────────────────────────────────────────────
// cURL write callback
// ─────────────────────────────────────────────────────────────────────────────
std::size_t BlenderFetcher::writeCallback(void* ptr, std::size_t size,
                                          std::size_t nmemb, std::string* buf)
{
    buf->append(static_cast<const char*>(ptr), size * nmemb);
    return size * nmemb;
}

// ─────────────────────────────────────────────────────────────────────────────
// HTTP GET
// ─────────────────────────────────────────────────────────────────────────────
std::string BlenderFetcher::httpGet(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init() failed");

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
// Parsing HTML – extrait les entrées "Blenderx.y/" de l'index Apache
// ─────────────────────────────────────────────────────────────────────────────
std::vector<BlenderVersion> BlenderFetcher::parseHtml(const std::string& html)
{
    std::vector<BlenderVersion> result;

    // L'index Apache contient des liens du type  href="Blender4.4/"
    std::regex re(R"(href="Blender(\d+\.\d+)/")");
    auto beg = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();

    for (auto it = beg; it != end; ++it) {
        std::string ver = (*it)[1].str();
        // Éviter les doublons
        bool dup = false;
        for (const auto& v : result) if (v.version == ver) { dup = true; break; }
        if (!dup) result.push_back({ ver, false });
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Comparaison sémantique "major.minor"
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

static bool verLess(const BlenderVersion& a, const BlenderVersion& b)
{
    auto va = splitVer(a.version);
    auto vb = splitVer(b.version);
    for (std::size_t i = 0; i < std::min(va.size(), vb.size()); ++i)
        if (va[i] != vb[i]) return va[i] < vb[i];
    return va.size() < vb.size();
}

// ─────────────────────────────────────────────────────────────────────────────
// fetch()  –  lance le téléchargement dans un thread séparé
// ─────────────────────────────────────────────────────────────────────────────
void BlenderFetcher::fetch()
{
    if (m_loading.load()) return;

    m_loading = true;
    m_failed  = false;

    m_future = std::async(std::launch::async, [this]()
    {
        try {
            curl_global_init(CURL_GLOBAL_DEFAULT);

            auto html     = httpGet(RELEASE_URL);
            auto versions = parseHtml(html);

            // Tri croissant par version
            std::sort(versions.begin(), versions.end(), verLess);

            // Marquer la dernière version
            if (!versions.empty())
                versions.back().isLatest = true;

            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_versions = std::move(versions);
                m_hasData  = true;
            }

            std::cout << "[BlenderFetcher] " << m_versions.size()
                      << " versions trouvees. Derniere : "
                      << m_versions.back().version << '\n';
        }
        catch (const std::exception& e) {
            std::cerr << "[BlenderFetcher] Erreur : " << e.what() << '\n';
            m_failed = true;
        }

        curl_global_cleanup();
        m_loading = false;
    });
}

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
