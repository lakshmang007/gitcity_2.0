#include "WorldGenerator.h"
#include <algorithm>
#include <iostream>
#include <cmath>

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
    int w  = (int)std::ceil(repo.width);
    int h  = (int)std::ceil(repo.height);
    if (w < 3) w = 3;
    if (h < 4) h = 4;
    
    for (int x = 0; x < w; x++) {
        for (int z = 0; z < w; z++) {
            // Foundation (2 layers of stone)
            setBlockWorld(startX + x, 0, startZ + z, STONE);
            setBlockWorld(startX + x, 1, startZ + z, STONE);
            
            // Walls with windows
            for (int y = 2; y < h; y++) {
                bool isEdge = (x == 0 || x == w-1 || z == 0 || z == w-1);
                if (isEdge) {
                    // Window pattern: every 3rd row, alternating columns
                    bool isWin = (y % 3 == 0) && ((x + z) % 2 == 0);
                    setBlockWorld(startX + x, y, startZ + z, isWin ? WINDOW : BRICK);
                } else {
                    // Interior floor every 3 blocks
                    if (y % 3 == 0)
                        setBlockWorld(startX + x, y, startZ + z, STONE);
                    // else AIR (interior gap)
                }
            }
            // Roof
            setBlockWorld(startX + x, h, startZ + z, METAL);
        }
    }
    
    // Beacon tower if open issues
    if (repo.hasBeacon) {
        int cx = startX + w/2;
        int cz = startZ + w/2;
        for (int b = 1; b <= 4; b++) {
            setBlockWorld(cx, h + b, cz, GLASS);
        }
        setBlockWorld(cx, h + 5, cz, WINDOW); // Glowing cap
    }
}

// Language to biome block type mapping
BlockType WorldGenerator::getBiomeBlock(const std::string& lang) {
    if (lang == "Python" || lang == "Jupyter Notebook") return GRASS;
    if (lang == "JavaScript" || lang == "TypeScript")    return ROAD;   // dark asphalt
    if (lang == "Java" || lang == "Kotlin")              return BRICK;
    if (lang == "C++" || lang == "C")                    return STONE;
    if (lang == "Rust" || lang == "Go")                  return METAL;
    if (lang == "Ruby")                                  return BRICK;
    if (lang == "HTML" || lang == "CSS")                 return WOOD;
    return GRASS; // default
}

void WorldGenerator::generateWorld(const std::vector<RepoData>& repoData) {
    repos = repoData;
    labels.clear();
    std::cout << "Generating Virtual Earth..." << std::endl;
    
    // Group by language
    std::map<std::string, std::vector<RepoData>> langGroups;
    for (const auto& r : repos) {
        langGroups[r.language].push_back(r);
    }
    
    int numCountries = langGroups.size();
    int countriesPerRow = std::max(1, (int)std::ceil(std::sqrt(numCountries)));
    int spacing = 120; // Space between country centers
    
    int count = 0;
    for (auto& [lang, langRepos] : langGroups) {
        int originX = (count % countriesPerRow) * spacing;
        int originZ = (count / countriesPerRow) * spacing;
        
        BlockType ground = getBiomeBlock(lang);
        
        // --- ISLAND BASE (80x80 blocks) ---
        int islandSize = 80;
        for (int x = 0; x < islandSize; x++) {
            for (int z = 0; z < islandSize; z++) {
                // Rounded island shape (ellipse)
                float dx = (x - islandSize/2.0f) / (islandSize/2.0f);
                float dz = (z - islandSize/2.0f) / (islandSize/2.0f);
                if (dx*dx + dz*dz > 1.0f) continue; // skip corners for round island
                
                setBlockWorld(originX + x, 0, originZ + z, ground);
                setBlockWorld(originX + x, -1, originZ + z, STONE);
            }
        }
        
        // Country label
        labels.push_back({ lang, glm::vec3(originX + islandSize/2, 50, originZ + islandSize/2), true });

        // --- PLACE BUILDINGS on the island ---
        int bx = 8, bz = 8; // building placement offset
        for (auto& repo : langRepos) {
            int w = (int)std::ceil(repo.width);
            
            // Record world position on the repo
            repo.worldPos = glm::vec3(originX + bx + w/2.0f, 0, originZ + bz + w/2.0f);
            
            placeBuilding(repo, originX + bx, originZ + bz);
            
            // Repo label (floating above building)
            labels.push_back({ repo.name, glm::vec3(originX + bx + w/2.0f, repo.height + 3.0f, originZ + bz + w/2.0f), false });

            bz += w + 5;
            if (bz + w > islandSize - 8) {
                bz = 8;
                bx += 18;
            }
            if (bx + w > islandSize - 8) break; // island full
        }
        count++;
    }

    // --- OCEAN: fill water around all islands ---
    // Determine bounding box of all islands
    int totalW = countriesPerRow * spacing + 80;
    int totalH = ((numCountries + countriesPerRow - 1) / countriesPerRow) * spacing + 80;
    int pad = 60;
    
    std::cout << "Generating ocean (" << (totalW + 2*pad) << "x" << (totalH + 2*pad) << " blocks)..." << std::endl;
    for (int x = -pad; x < totalW + pad; x++) {
        for (int z = -pad; z < totalH + pad; z++) {
            // Only place water if there's no land above
            glm::ivec3 cpos;
            // Check if y=0 has a block already (quick chunk lookup)
            int cx = (x >= 0) ? x / Chunk::SIZE : (x - Chunk::SIZE + 1) / Chunk::SIZE;
            int cz = (z >= 0) ? z / Chunk::SIZE : (z - Chunk::SIZE + 1) / Chunk::SIZE;
            glm::ivec3 chunkKey(cx * Chunk::SIZE, 0, cz * Chunk::SIZE);
            
            bool hasLand = false;
            auto it = chunks.find(chunkKey);
            if (it != chunks.end()) {
                int lx = x - chunkKey.x;
                int lz = z - chunkKey.z;
                if (lx >= 0 && lx < Chunk::SIZE && lz >= 0 && lz < Chunk::SIZE) {
                    if (it->second->getBlock(lx, 0, lz) != AIR) {
                        hasLand = true;
                    }
                }
            }
            
            if (!hasLand) {
                setBlockWorld(x, -1, z, WATER);
            }
        }
    }
    
    // Generate all chunk meshes
    std::cout << "Building meshes for " << chunks.size() << " chunks..." << std::endl;
    for (auto& [pos, chunk] : chunks) {
        chunk->generateMesh();
    }
    std::cout << "World generation complete!" << std::endl;
}

void WorldGenerator::draw(const glm::mat4& view, const glm::mat4& projection) {
    for (auto const& [pos, chunk] : chunks) {
        chunk->draw();
    }
}
