#include "GitHubAPI.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

using json = nlohmann::json;

size_t GitHubAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string GitHubAPI::performGetRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "GitCity-App");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::vector<RepoData> GitHubAPI::fetchUserRepos(const std::string& username) {
    std::string url = "https://api.github.com/users/" + username + "/repos?per_page=100&sort=updated";
    std::string response = performGetRequest(url);
    std::cout << "Response received (" << response.length() << " bytes). Parsing..." << std::endl;
    
    std::vector<RepoData> repos;

    auto safe_string = [](const json& j, const std::string& key, const std::string& def) -> std::string {
        if (j.contains(key) && j[key].is_string()) return j[key].get<std::string>();
        return def;
    };

    try {
        auto j = json::parse(response);
        if (j.is_array()) {
            for (auto& item : j) {
                RepoData repo;
                repo.name        = safe_string(item, "name", "Unknown");
                repo.description = safe_string(item, "description", "");
                repo.language    = safe_string(item, "language", "Unknown");
                repo.stars       = item.value("stargazers_count", 0);
                repo.forks       = item.value("forks_count", 0);
                repo.openIssues  = item.value("open_issues_count", 0);
                repo.size        = item.value("size", 0);
                repo.updatedAt   = safe_string(item, "pushed_at", "");

                if (item.contains("topics") && item["topics"].is_array()) {
                    for (auto& topic : item["topics"]) {
                        if (topic.is_string()) repo.topics.push_back(topic.get<std::string>());
                    }
                }

                // --- SCALE buildings to be VISIBLE ---
                // Height: min 4 blocks, logarithmic from size, capped at 40
                repo.height = std::max(4.0f, std::min(40.0f, (float)std::log(repo.size + 10.0f) * 3.5f));
                // Width: min 3 blocks, based on stars/forks, capped at 12
                repo.width  = std::max(3.0f, std::min(12.0f, 3.0f + (float)std::sqrt(repo.stars + repo.forks + 1.0f)));

                // Color: warm = recent, cool = old (placeholder: all get a hue)
                float hue = std::fmod(std::hash<std::string>{}(repo.language) * 0.618033988f, 1.0f);
                repo.color = glm::vec3(
                    0.5f + 0.4f * std::cos(6.28318f * hue),
                    0.5f + 0.4f * std::cos(6.28318f * (hue - 0.33f)),
                    0.5f + 0.4f * std::cos(6.28318f * (hue - 0.66f))
                );

                repo.hasBeacon = (repo.openIssues > 0);
                repos.push_back(repo);
            }
            std::cout << "Parsed " << repos.size() << " repositories." << std::endl;
        } else if (j.contains("message")) {
            std::cerr << "GitHub API Error: " << j["message"] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
    }
    
    return repos;
}
