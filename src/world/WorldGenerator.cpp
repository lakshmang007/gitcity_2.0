#include "WorldGenerator.h"
#include <algorithm>
#include <iostream>

bool compareVec3(const glm::ivec3& a, const glm::ivec3& b) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.z < b.z;
}

WorldGenerator::WorldGenerator() : chunks(compareVec3) {}

void WorldGenerator::setBlockWorld(int x, int y, int z, BlockType type) {
    int cx = (x >= 0) ? x / Chunk::SIZE : (x - Chunk::SIZE + 1) / Chunk::SIZE;
    int cy = (y >= 0) ? y / Chunk::HEIGHT : (y - Chunk::HEIGHT + 1) / Chunk::HEIGHT;
    int cz = (z >= 0) ? z / Chunk::SIZE : (z - Chunk::SIZE + 1) / Chunk::SIZE;
    
    glm::ivec3 cpos(cx * Chunk::SIZE, cy * Chunk::HEIGHT, cz * Chunk::SIZE);
    if (chunks.find(cpos) == chunks.end()) {
        chunks[cpos] = std::make_unique<Chunk>(cpos);
    }
    
    int lx = x - cpos.x;
    int ly = y - cpos.y;
    int lz = z - cpos.z;
    chunks[cpos]->setBlock(lx, ly, lz, type);
}

void WorldGenerator::placeBuilding(const RepoData& repo, int startX, int startZ) {
    int width = (int)std::ceil(repo.width);
    int height = (int)std::ceil(repo.height);
    
    // Base/Footprint
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < width; z++) {
            // Foundation
            setBlockWorld(startX + x, 0, startZ + z, STONE);
            // Walls - with windows
            for (int y = 1; y < height; y++) {
                if (x == 0 || x == width - 1 || z == 0 || z == width - 1) {
                    bool isWindow = ((y % 3 == 0) && ((x + z) % 2 == 0)); // Stylized pattern
                    setBlockWorld(startX + x, y, startZ + z, isWindow ? WINDOW : BRICK);
                } else {
                    setBlockWorld(startX + x, y, startZ + z, STONE); // Core
                }
            }
            // Roof
            setBlockWorld(startX + x, height, startZ + z, METAL);
        }
    }
    
    // Glowing beacon if open issues (Git-City reference)
    if (repo.hasBeacon) {
        for (int b = 1; b < 5; b++) {
             setBlockWorld(startX + width/2, height + b, startZ + width/2, GLASS); 
        }
        setBlockWorld(startX + width/2, height + 5, startZ + width/2, WINDOW); // Glowing cap
    }
}

void WorldGenerator::generateWorld(const std::vector<RepoData>& repoData) {
    repos = repoData;
    labels.clear();
    
    // Group by language (each is a country/island)
    std::map<std::string, std::vector<RepoData>> langGroups;
    for (const auto& r : repos) {
        langGroups[r.language].push_back(r);
    }
    
    int countryX = 0;
    int countryZ = 0;
    int countriesPerRow = (int)std::sqrt(langGroups.size());
    if (countriesPerRow < 1) countriesPerRow = 1;
    
    int count = 0;
    for (auto const& [lang, langRepos] : langGroups) {
        // Space countries much further apart (e.g. 1000 units)
        int startX = (count % countriesPerRow) * 1000;
        int startZ = (count / countriesPerRow) * 1000;
        
        // ISLAND Base
        for (int x = -20; x < 80; x++) {
            for (int z = -20; z < 80; z++) {
                setBlockWorld(startX + x, -1, startZ + z, GRASS);
            }
        }
        
        // Country Label (visible from height)
        labels.push_back({ "The Land of " + lang, glm::vec3(startX + 30, 40, startZ + 30) });

        // Place buildings within the country
        int curX_off = 5;
        int curZ_off = 5;
        for (const auto& repo : langRepos) {
            placeBuilding(repo, startX + curX_off, startZ + curZ_off);
            
            // Track building labels (only visible zoomed in)
            labels.push_back({ repo.name, glm::vec3(startX + curX_off + repo.width/2.0f, repo.height + 2.0f, startZ + curZ_off + repo.width/2.0f) });

            curZ_off += (int)repo.width + 12;
            if (curZ_off > 60) {
                curZ_off = 5;
                curX_off += 25;
            }
        }
        count++;
    }

    // Add a GIANT Water Floor (The "Ocean")
    for (int x = -2000; x < 5000; x += 32) {
        for (int z = -2000; z < 5000; z += 32) {
             setBlockWorld(x, -2, z, WATER); 
        }
    }
}

void WorldGenerator::draw(const glm::mat4& view, const glm::mat4& projection) {
    for (auto const& [pos, chunk] : chunks) {
        chunk->draw();
    }
}
