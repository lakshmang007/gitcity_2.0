#include "WorldGenerator.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool compareVec3(const glm::ivec3& a, const glm::ivec3& b) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.z < b.z;
}

WorldGenerator::WorldGenerator() : chunks(compareVec3) {}

// Simple 2D noise from sine combinations for organic continent coastlines
float WorldGenerator::noise2D(float x, float z) {
    return (std::sin(x * 0.1f + z * 0.13f) * 0.5f +
            std::sin(x * 0.23f - z * 0.17f) * 0.3f +
            std::sin(x * 0.07f + z * 0.31f) * 0.2f +
            std::cos(x * 0.19f + z * 0.11f) * 0.25f +
            std::sin(x * 0.41f - z * 0.29f) * 0.15f);
}

std::string WorldGenerator::getFantasyName(const std::string& lang) {
    static std::map<std::string, std::string> names = {
        {"Python", "Pythonia"},
        {"JavaScript", "Scriptoria"},
        {"TypeScript", "Typescriptia"},
        {"Java", "Javaria"},
        {"C++", "Ceplusia"},
        {"C", "Celanor"},
        {"C#", "Sharpenia"},
        {"Go", "Golandia"},
        {"Rust", "Rustland"},
        {"Ruby", "Rubylon"},
        {"PHP", "Phpyria"},
        {"Swift", "Swiftara"},
        {"Kotlin", "Kotlinheim"},
        {"Dart", "Dartmoor"},
        {"Scala", "Scalania"},
        {"Lua", "Luaria"},
        {"Shell", "Shellshore"},
        {"HTML", "Htmlia"},
        {"CSS", "Cssenia"},
        {"R", "Rathenia"},
        {"Perl", "Perlmont"},
        {"Haskell", "Haskellum"},
        {"Elixir", "Elixiria"},
        {"Jupyter Notebook", "Jupyterra"},
        {"Vue", "Vueland"},
        {"Dockerfile", "Dockeria"},
        {"Unknown", "Terra Incognita"},
    };
    auto it = names.find(lang);
    if (it != names.end()) return it->second;
    // Generate a name for unknown languages
    return lang + "ia";
}

glm::vec3 WorldGenerator::getCountryColor(int index) {
    // Rich, distinct, saturated colors for each country
    static std::vector<glm::vec3> palette = {
        {0.22f, 0.65f, 0.35f},  // Emerald green
        {0.85f, 0.45f, 0.20f},  // Warm orange
        {0.35f, 0.50f, 0.82f},  // Royal blue
        {0.78f, 0.25f, 0.35f},  // Crimson
        {0.60f, 0.40f, 0.75f},  // Purple
        {0.90f, 0.72f, 0.20f},  // Gold
        {0.30f, 0.70f, 0.70f},  // Teal
        {0.85f, 0.35f, 0.60f},  // Rose
        {0.45f, 0.75f, 0.30f},  // Lime
        {0.70f, 0.50f, 0.30f},  // Bronze
        {0.40f, 0.40f, 0.85f},  // Indigo
        {0.88f, 0.55f, 0.45f},  // Salmon
        {0.25f, 0.60f, 0.55f},  // Sea green
        {0.75f, 0.65f, 0.50f},  // Tan
        {0.55f, 0.30f, 0.65f},  // Violet
        {0.80f, 0.80f, 0.30f},  // Yellow-green
    };
    return palette[index % palette.size()];
}

BlockType WorldGenerator::getGroundBlock(int index) {
    // Vary the ground block type for visual variety
    static BlockType blocks[] = { GRASS, STONE, WOOD, BRICK, ROAD, METAL, LEAVES };
    return blocks[index % 7];
}

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

void WorldGenerator::placeBuilding(const RepoData& repo, int startX, int startZ, BlockType wallType) {
    int w = (int)std::ceil(repo.width);
    int h = (int)std::ceil(repo.height);
    if (w < 3) w = 3;
    if (h < 4) h = 4;
    
    for (int x = 0; x < w; x++) {
        for (int z = 0; z < w; z++) {
            // Foundation
            setBlockWorld(startX + x, 1, startZ + z, STONE);
            setBlockWorld(startX + x, 2, startZ + z, STONE);
            
            // Walls with windows
            for (int y = 3; y < h + 1; y++) {
                bool isEdge = (x == 0 || x == w-1 || z == 0 || z == w-1);
                if (isEdge) {
                    bool isWin = (y % 3 == 0) && ((x + z) % 2 == 0);
                    setBlockWorld(startX + x, y, startZ + z, isWin ? WINDOW : wallType);
                } else {
                    if (y % 4 == 0) setBlockWorld(startX + x, y, startZ + z, STONE);
                }
            }
            // Roof
            setBlockWorld(startX + x, h + 1, startZ + z, METAL);
        }
    }
    
    // Beacon for repos with open issues
    if (repo.hasBeacon) {
        int cx = startX + w/2, cz = startZ + w/2;
        for (int b = 1; b <= 3; b++)
            setBlockWorld(cx, h + 1 + b, cz, GLASS);
        setBlockWorld(cx, h + 5, cz, WINDOW);
    }
}

void WorldGenerator::generateWorld(const std::vector<RepoData>& repoData) {
    labels.clear();
    countries.clear();
    std::cout << "=== Generating Fantasy World Map ===" << std::endl;
    
    // --- Group repos by language ---
    std::map<std::string, std::vector<RepoData>> langGroups;
    for (const auto& r : repoData) {
        langGroups[r.language].push_back(r);
    }
    
    int numCountries = langGroups.size();
    if (numCountries == 0) return;
    
    // Total repos for proportional sizing
    int totalRepos = repoData.size();
    
    // --- Compute layout: arrange countries in a circle ---
    // Map radius scales with total countries
    float mapRadius = 40.0f + numCountries * 30.0f;
    float angleStep = 2.0f * M_PI / numCountries;
    
    int idx = 0;
    for (auto& [lang, langRepos] : langGroups) {
        CountryInfo country;
        country.language = lang;
        country.fantasyName = getFantasyName(lang);
        country.color = getCountryColor(idx);
        country.repoCount = langRepos.size();
        country.repos = langRepos;
        
        // Country radius proportional to repo count (min 25, max 60)
        float proportion = (float)langRepos.size() / totalRepos;
        country.radius = std::max(25.0f, std::min(60.0f, 25.0f + proportion * 80.0f));
        
        // Position on the circular layout
        float angle = angleStep * idx;
        float dist = mapRadius;
        country.center = glm::vec2(
            dist * std::cos(angle),
            dist * std::sin(angle)
        );
        
        countries.push_back(country);
        idx++;
    }
    
    // --- Convert center coordinates to integer world coords ---
    // Shift everything so minimum is (10, 10)
    float minX = 1e9, minZ = 1e9;
    for (auto& c : countries) {
        minX = std::min(minX, c.center.x - c.radius - 20);
        minZ = std::min(minZ, c.center.y - c.radius - 20);
    }
    float offsetX = -minX + 10;
    float offsetZ = -minZ + 10;
    for (auto& c : countries) {
        c.center.x += offsetX;
        c.center.y += offsetZ;
    }
    
    // --- Determine world bounds ---
    float maxX = 0, maxZ = 0;
    for (auto& c : countries) {
        maxX = std::max(maxX, c.center.x + c.radius + 30);
        maxZ = std::max(maxZ, c.center.y + c.radius + 30);
    }
    int worldW = (int)maxX + 20;
    int worldH = (int)maxZ + 20;
    
    std::cout << "World size: " << worldW << "x" << worldH << " blocks" << std::endl;
    std::cout << "Countries: " << numCountries << std::endl;
    
    // --- GENERATE TERRAIN ---
    // For each block in the world, determine if it's land (which country) or ocean
    for (int x = 0; x < worldW; x++) {
        for (int z = 0; z < worldH; z++) {
            
            // Check if this point belongs to any country
            int bestCountry = -1;
            float bestDist = 1e9;
            
            for (int c = 0; c < (int)countries.size(); c++) {
                float dx = x - countries[c].center.x;
                float dz = z - countries[c].center.y;
                float dist = std::sqrt(dx*dx + dz*dz);
                
                // Organic coastline: modify radius with noise
                float angle = std::atan2(dz, dx);
                float noiseFactor = noise2D(x * 0.15f, z * 0.15f) * 0.3f;
                float effectiveRadius = countries[c].radius * (1.0f + noiseFactor);
                // Add "peninsulas" using additional noise harmonics
                effectiveRadius += countries[c].radius * 0.15f * std::sin(angle * 3.0f + c * 1.7f);
                effectiveRadius += countries[c].radius * 0.10f * std::sin(angle * 5.0f + c * 2.3f);
                
                if (dist < effectiveRadius) {
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestCountry = c;
                    }
                }
            }
            
            if (bestCountry >= 0) {
                // LAND: place ground blocks
                BlockType ground = getGroundBlock(bestCountry);
                setBlockWorld(x, 0, z, ground);
                // Sub-surface layer
                setBlockWorld(x, -1, z, STONE);
                
                // Occasional terrain features (hills)
                float hillNoise = noise2D(x * 0.08f, z * 0.08f);
                if (hillNoise > 0.5f) {
                    setBlockWorld(x, 1, z, ground);
                }
            } else {
                // OCEAN
                setBlockWorld(x, -1, z, WATER);
            }
        }
    }
    
    // --- PLACE BUILDINGS on each country ---
    for (int c = 0; c < (int)countries.size(); c++) {
        auto& country = countries[c];
        int cx = (int)country.center.x;
        int cz = (int)country.center.y;
        
        // Country name label (floating high above)
        std::string countryLabel = country.fantasyName + " (" + std::to_string(country.repoCount) + " repos)";
        labels.push_back({ countryLabel, glm::vec3(cx, 55, cz), true, country.color });
        
        // Place buildings in a grid within the country
        int numRepos = country.repos.size();
        int gridSize = (int)std::ceil(std::sqrt(numRepos));
        float spacing = std::min(12.0f, country.radius * 0.6f / std::max(1, gridSize));
        
        float startBX = cx - (gridSize * spacing) / 2.0f;
        float startBZ = cz - (gridSize * spacing) / 2.0f;
        
        BlockType wallType = BRICK; // vary by country
        if (c % 3 == 1) wallType = STONE;
        if (c % 3 == 2) wallType = METAL;
        
        for (int i = 0; i < numRepos; i++) {
            auto& repo = country.repos[i];
            int bx = (int)(startBX + (i % gridSize) * spacing);
            int bz = (int)(startBZ + (i / gridSize) * spacing);
            
            // Verify this position is on land (within country radius)
            float dx = bx - cx;
            float dz = bz - cz;
            float dist = std::sqrt(dx*dx + dz*dz);
            if (dist > country.radius * 0.7f) {
                // Move closer to center
                bx = cx + (int)(dx * 0.5f);
                bz = cz + (int)(dz * 0.5f);
            }
            
            placeBuilding(repo, bx, bz, wallType);
            
            // Building label
            int bh = (int)std::ceil(repo.height);
            labels.push_back({ repo.name, glm::vec3(bx + repo.width/2.0f, bh + 4.0f, bz + repo.width/2.0f), false, glm::vec3(1.0f) });
        }
        
        std::cout << "  " << country.fantasyName << " (" << country.language << "): " 
                  << country.repoCount << " repos, radius=" << (int)country.radius << std::endl;
    }
    
    // --- Build all chunk meshes ---
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
