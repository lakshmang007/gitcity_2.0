#include "Chunk.h"
#include <vector>

Chunk::Chunk(glm::ivec3 pos) : position(pos), VAO(0), VBO(0), EBO(0), indexCount(0), needsUpdate(true) {
    for(int x = 0; x < SIZE; x++)
        for(int y = 0; y < HEIGHT; y++)
            for(int z = 0; z < SIZE; z++)
                blocks[x][y][z] = AIR;
}

Chunk::~Chunk() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (x >= 0 && x < SIZE && y >= 0 && y < HEIGHT && z >= 0 && z < SIZE) {
        blocks[x][y][z] = (unsigned char)type;
        needsUpdate = true;
    }
}

BlockType Chunk::getBlock(int x, int y, int z) {
    if (x >= 0 && x < SIZE && y >= 0 && y < HEIGHT && z >= 0 && z < SIZE) {
        return (BlockType)blocks[x][y][z];
    }
    return AIR;
}

void Chunk::generateMesh() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    auto addFace = [&](glm::vec3 p, int faceDir, BlockType type) {
        // Simple cube face vertices
        // faceDir: 0:front, 1:back, 2:left, 3:right, 4:top, 5:bottom
        glm::vec3 normal;
        glm::vec3 v[4];
        if (faceDir == 0) { // Front
            normal = {0, 0, 1};
            v[0] = {0, 0, 1}; v[1] = {1, 0, 1}; v[2] = {1, 1, 1}; v[3] = {0, 1, 1};
        } else if (faceDir == 1) { // Back
            normal = {0, 0, -1};
            v[0] = {1, 0, 0}; v[1] = {0, 0, 0}; v[2] = {0, 1, 0}; v[3] = {1, 1, 0};
        } else if (faceDir == 2) { // Left
            normal = {-1, 0, 0};
            v[0] = {0, 0, 0}; v[1] = {0, 0, 1}; v[2] = {0, 1, 1}; v[3] = {0, 1, 0};
        } else if (faceDir == 3) { // Right
            normal = {1, 0, 0};
            v[0] = {1, 0, 1}; v[1] = {1, 0, 0}; v[2] = {1, 1, 0}; v[3] = {1, 1, 1};
        } else if (faceDir == 4) { // Top
            normal = {0, 1, 0};
            v[0] = {0, 1, 1}; v[1] = {1, 1, 1}; v[2] = {1, 1, 0}; v[3] = {0, 1, 0};
        } else { // Bottom
            normal = {0, -1, 0};
            v[0] = {0, 0, 0}; v[1] = {1, 0, 0}; v[2] = {1, 0, 1}; v[3] = {0, 0, 1};
        }

        unsigned int startIdx = vertices.size();
        for (int i = 0; i < 4; i++) {
            Vertex vert;
            vert.position = p + v[i];
            vert.normal = normal;
            vert.texCoords = (i == 0) ? glm::vec2(0, 0) : (i == 1) ? glm::vec2(1, 0) : (i == 2) ? glm::vec2(1, 1) : glm::vec2(0, 1);
            vert.blockType = (int)type;
            vertices.push_back(vert);
        }
        indices.push_back(startIdx); indices.push_back(startIdx + 1); indices.push_back(startIdx + 2);
        indices.push_back(startIdx); indices.push_back(startIdx + 2); indices.push_back(startIdx + 3);
    };

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int z = 0; z < SIZE; z++) {
                BlockType type = getBlock(x, y, z);
                if (type == AIR) continue;

                glm::vec3 p(x + position.x, y + position.y, z + position.z);

                // Check neighbors
                if (z + 1 >= SIZE || getBlock(x, y, z + 1) == AIR) addFace(p, 0, type);
                if (z - 1 < 0 || getBlock(x, y, z - 1) == AIR) addFace(p, 1, type);
                if (x - 1 < 0 || getBlock(x - 1, y, z) == AIR) addFace(p, 2, type);
                if (x + 1 >= SIZE || getBlock(x + 1, y, z) == AIR) addFace(p, 3, type);
                if (y + 1 >= HEIGHT || getBlock(x, y + 1, z) == AIR) addFace(p, 4, type);
                if (y - 1 < 0 || getBlock(x, y - 1, z) == AIR) addFace(p, 5, type);
            }
        }
    }

    indexCount = indices.size();
    if (indexCount == 0) return;

    if (!VAO) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    // BlockType
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, blockType));
    glEnableVertexAttribArray(3);

    needsUpdate = false;
}

void Chunk::draw() {
    if (needsUpdate) generateMesh();
    if (indexCount > 0) {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
}
