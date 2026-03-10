#ifndef BUILDING_H
#define BUILDING_H

#include <glm/glm.hpp>
#include "../github/RepoData.h"

class Building {
public:
    RepoData data;
    glm::vec3 position;
    glm::vec3 dimensions;
    bool isPlayerInside;

    Building(RepoData d, glm::vec3 pos) : data(d), position(pos), isPlayerInside(false) {
        dimensions = glm::vec3(d.width, d.height, d.width);
    }

    bool checkCollision(glm::vec3 playerPos) {
        return (playerPos.x >= position.x && playerPos.x <= position.x + dimensions.x &&
                playerPos.z >= position.z && playerPos.z <= position.z + dimensions.z &&
                playerPos.y >= position.y && playerPos.y <= position.y + dimensions.y);
    }
};

#endif
