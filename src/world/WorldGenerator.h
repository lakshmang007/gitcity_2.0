#ifndef WORLDGENERATOR_H
#define WORLDGENERATOR_H

#include <vector>
#include <map>
#include <memory>
#include "Chunk.h"
#include "../github/RepoData.h"

class WorldGenerator {
public:
    std::map<glm::ivec3, std::unique_ptr<Chunk>, bool(*)(const glm::ivec3&, const glm::ivec3&)> chunks;
    std::vector<RepoData> repos;

    struct Label {
        std::string text;
        glm::vec3 pos;
    };
    std::vector<Label> labels;

    WorldGenerator();
    void generateWorld(const std::vector<RepoData>& repoData);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    const std::vector<Label>& getLabels() const { return labels; }

private:
    void placeBuilding(const RepoData& repo, int startX, int startZ);
    void setBlockWorld(int x, int y, int z, BlockType type);
};

#endif
