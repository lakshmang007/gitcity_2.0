#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>

enum BlockType {
    AIR = 0,
    GRASS,
    STONE,
    WOOD,
    LEAVES,
    WATER,
    WINDOW,
    BRICK,
    GLASS,
    METAL,
    ROAD
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    int blockType;
};

class Chunk {
public:
    static const int SIZE = 16;
    static const int HEIGHT = 64;
    
    unsigned char blocks[SIZE][HEIGHT][SIZE];
    glm::ivec3 position; // Chunk position in world coordinates

    unsigned int VAO, VBO, EBO;
    int indexCount;
    bool needsUpdate;

    Chunk(glm::ivec3 pos);
    ~Chunk();

    void generateMesh();
    void draw();
    void setBlock(int x, int y, int z, BlockType type);
    BlockType getBlock(int x, int y, int z);
};

#endif
