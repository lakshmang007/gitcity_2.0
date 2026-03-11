#ifndef REPODATA_H
#define REPODATA_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct RepoData {
    std::string name = "";
    std::string description = "";
    std::string language = "Unknown";
    int stars = 0;
    int forks = 0;
    int openIssues = 0;
    int size = 0;
    std::string updatedAt = "";
    std::vector<std::string> topics;

    // Computed visual properties
    float height = 3.0f;   // Building height in blocks
    float width  = 2.0f;   // Building footprint in blocks  
    glm::vec3 color = glm::vec3(0.5f);
    bool hasBeacon = false;

    // World position (set by WorldGenerator)
    glm::vec3 worldPos = glm::vec3(0.0f);
};

#endif
