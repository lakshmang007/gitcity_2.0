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
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "GitCity-App"); // GitHub requires user agent
        
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::vector<RepoData> GitHubAPI::fetchUserRepos(const std::string& username) {
    std::string url = "https://api.github.com/users/" + username + "/repos?per_page=100";
    std::string response = performGetRequest(url);
    std::cout << "Response received (" << response.length() << " bytes). Parsing JSON..." << std::endl;
    
    std::vector<RepoData> repos;
    try {
        auto safe_string = [](const json& j, const std::string& key, const std::string& defaultValue) {
            if (j.contains(key) && j[key].is_string()) {
                return j[key].get<std::string>();
            }
            return defaultValue;
        };

        auto j = json::parse(response);
        if (j.is_array()) {
            for (auto& item : j) {
                RepoData repo;
                repo.name = safe_string(item, "name", "Unknown");
                repo.description = safe_string(item, "description", "No description provided");
                repo.language = safe_string(item, "language", "Unknown Territories");
                repo.stars = item.value("stargazers_count", 0);
                repo.forks = item.value("forks_count", 0);
                repo.openIssues = item.value("open_issues_count", 0);
                repo.size = item.value("size", 0);
                repo.updatedAt = safe_string(item, "pushed_at", "");
                
                if (item.contains("topics") && item["topics"].is_array()) {
                    for (auto& topic : item["topics"]) {
                        if (topic.is_string()) {
                            repo.topics.push_back(topic.get<std::string>());
                        }
                    }
                }

                // Calculate dimensions
                // Height = log(size + 1) * factor
                repo.height = std::max(2.0f, (float)std::log(repo.size + 1.0f) * 2.0f);
                // Width = sqrt(stars + 1) * factor
                repo.width = std::max(1.0f, (float)std::sqrt(repo.stars + 1.0f) * 0.5f);
                
                // Color based on recency (placeholder logic)
                // For now, let's just use some varied colors
                repo.color = glm::vec3(0.1f, 0.5f, 0.8f); // Blue
                if (repo.openIssues > 0) repo.hasBeacon = true;
                else repo.hasBeacon = false;

                repos.push_back(repo);
                std::cout << "Parsed " << repos.size() << ": " << repo.name << std::endl;
            }
            std::cout << "Total parsed: " << repos.size() << std::endl;
        } else if (j.contains("message")) {
            std::cerr << "GitHub API Error: " << j["message"] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
    }
    
    return repos;
}
