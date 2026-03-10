#ifndef REPODATA_H
#define REPODATA_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct RepoData {
    std::string name;
    std::string description;
    std::string language;
    int stars = 0;
    int forks = 0;
    int openIssues = 0;
    long long size = 0;
    std::string updatedAt;
    std::vector<std::string> topics;
    int commitCount = 0;
    
    // Derived dimensions
    float height = 3.0f;
    float width = 2.0f;
    glm::vec3 color = glm::vec3(0.5f);
    bool hasBeacon = false;
};

#endif
