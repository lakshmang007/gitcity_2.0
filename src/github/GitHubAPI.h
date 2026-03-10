#ifndef GITHUBAPI_H
#define GITHUBAPI_H

#include <string>
#include <vector>
#include "RepoData.h"

class GitHubAPI {
public:
    static std::vector<RepoData> fetchUserRepos(const std::string& username);
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static std::string performGetRequest(const std::string& url);
};

#endif
