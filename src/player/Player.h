#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "Camera.h"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Player {
public:
    Camera& camera;
    glm::vec3 velocity;
    bool isGrounded;
    float height = 1.8f;
    float radius = 0.4f;

    Player(Camera& cam) : camera(cam), velocity(0.0f), isGrounded(false) {}

    void update(float deltaTime) {
        // Simple gravity
        if (!isGrounded) {
            velocity.y -= 9.8f * deltaTime;
        } else if (velocity.y < 0) {
            velocity.y = 0;
        }

        camera.Position += velocity * deltaTime;
        
        // Placeholder ground collision
        if (camera.Position.y < height) {
            camera.Position.y = height;
            velocity.y = 0;
            isGrounded = true;
        } else {
            isGrounded = false;
        }
    }

    void handleInput(Camera_Movement direction, float deltaTime) {
        float speed = 5.0f;
        if (direction == FORWARD) {
            glm::vec3 front = camera.Front;
            front.y = 0; // Move only on XZ plane
            camera.Position += glm::normalize(front) * speed * deltaTime;
        }
        if (direction == BACKWARD) {
            glm::vec3 front = camera.Front;
            front.y = 0;
            camera.Position -= glm::normalize(front) * speed * deltaTime;
        }
        if (direction == LEFT) {
            camera.Position -= camera.Right * speed * deltaTime;
        }
        if (direction == RIGHT) {
            camera.Position += camera.Right * speed * deltaTime;
        }
    }

    void jump() {
        if (isGrounded) {
            velocity.y = 5.0f;
            isGrounded = false;
        }
    }
};

#endif
