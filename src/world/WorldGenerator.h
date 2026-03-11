#ifndef WORLDGENERATOR_H
#define WORLDGENERATOR_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include "Chunk.h"
#include "../github/RepoData.h"

struct CountryInfo {
    std::string language;       // Original language name
    std::string fantasyName;    // Fantasy country name
    glm::vec3 color;            // Distinct country color
    int repoCount;
    glm::vec2 center;           // Center position on map
    float radius;               // Base radius (varies by repo count)
    std::vector<RepoData> repos;
};

class WorldGenerator {
public:
    std::map<glm::ivec3, std::unique_ptr<Chunk>, bool(*)(const glm::ivec3&, const glm::ivec3&)> chunks;

    struct Label {
        std::string text;
        glm::vec3 pos;
        bool isCountry;
        glm::vec3 color;
    };
    std::vector<Label> labels;
    std::vector<CountryInfo> countries;

    WorldGenerator();
    void generateWorld(const std::vector<RepoData>& repoData);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    const std::vector<Label>& getLabels() const { return labels; }
    const std::vector<CountryInfo>& getCountries() const { return countries; }

private:
    void placeBuilding(const RepoData& repo, int startX, int startZ, BlockType wallType);
    void setBlockWorld(int x, int y, int z, BlockType type);
    
    std::string getFantasyName(const std::string& lang);
    glm::vec3 getCountryColor(int index);
    float noise2D(float x, float z);
    BlockType getGroundBlock(int index);
};

#endif
