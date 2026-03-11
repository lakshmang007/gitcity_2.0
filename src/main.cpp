#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "render/Shader.h"
#include "player/Camera.h"
#include "player/Player.h"
#include "github/GitHubAPI.h"
#include "world/WorldGenerator.h"
#include "ui/BuildingUI.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera & Player
Camera camera(glm::vec3(60.0f, 80.0f, 60.0f));
Player player(camera);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// View state
bool isGlobeView = true;
float flySpeed = 50.0f;  // Much faster for large worlds!

float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = flySpeed * deltaTime;
    
    // In globe view: always fly freely (no gravity)
    // Compute horizontal forward (ignore pitch for XZ movement)
    glm::vec3 flatFront = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position += flatFront * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Position -= flatFront * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Position -= camera.Right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Position += camera.Right * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Position.y += speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Position.y -= speed;
    
    // Speed control
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        flySpeed = std::max(5.0f, flySpeed - 0.5f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        flySpeed = std::min(200.0f, flySpeed + 0.5f);
        
    // Prevent going below water
    if (camera.Position.y < 1.0f) camera.Position.y = 1.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Scroll = move up/down quickly (zoom)
    camera.Position.y -= yoffset * 10.0f;
    if (camera.Position.y < 1.0f) camera.Position.y = 1.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // --- USERNAME INPUT ---
    std::string input;
    std::cout << "=== GitCity: Virtual Earth ===" << std::endl;
    std::cout << "Enter GitHub username or profile link: " << std::flush;
    std::cin >> input;

    std::string username = input;
    size_t pos = input.find("github.com/");
    if (pos != std::string::npos) {
        username = input.substr(pos + 11);
        size_t slash = username.find("/");
        if (slash != std::string::npos) username = username.substr(0, slash);
    }

    // --- FETCH DATA ---
    std::cout << "Fetching repositories for " << username << "..." << std::endl;
    std::vector<RepoData> repos = GitHubAPI::fetchUserRepos(username);
    if (repos.empty()) {
        std::cerr << "No repositories found. Creating demo world." << std::endl;
        RepoData dummy; dummy.name = "example-repo"; dummy.language = "Unknown";
        dummy.height = 8; dummy.width = 4;
        repos.push_back(dummy);
    }

    // --- INIT GLFW/OPENGL ---
    if (!glfwInit()) { std::cerr << "GLFW init failed" << std::endl; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GitCity - Virtual Earth", NULL, NULL);
    if (!window) { std::cerr << "Window creation failed" << std::endl; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { 
        std::cerr << "GLAD init failed" << std::endl; return -1; 
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    BuildingUI::init(window);

    // --- SHADERS ---
    Shader blockShader("shaders/block.vert", "shaders/block.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    // --- SKYBOX VAO ---
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // --- GENERATE WORLD ---
    WorldGenerator world;
    world.generateWorld(repos);
    
    // Set initial camera above the center of the world
    camera.Position = glm::vec3(60.0f, 120.0f, 60.0f);
    camera.Pitch = -45.0f;
    camera.Yaw = -90.0f;
    camera.MovementSpeed = flySpeed;
    camera.updateCameraVectors();
    
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "WASD = Fly | Space/Shift = Up/Down | Mouse = Look" << std::endl;
    std::cout << "Scroll = Quick zoom | Q/E = Slow/Fast speed" << std::endl;
    std::cout << "ESC = Exit" << std::endl;
    std::cout << "================\n" << std::endl;

    // --- MAIN LOOP ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear
        glClearColor(0.55f, 0.70f, 0.85f, 1.0f); // Match fog color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 
            0.5f, 2000.0f  // MUCH larger far plane
        );
        glm::mat4 view = camera.GetViewMatrix();

        // --- SKYBOX (rendered first, depth = far) ---
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view))); // Remove translation
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // --- VOXEL WORLD ---
        blockShader.use();
        blockShader.setMat4("projection", projection);
        blockShader.setMat4("view", view);
        blockShader.setMat4("model", glm::mat4(1.0f));
        blockShader.setVec3("viewPos", camera.Position);  // THIS WAS MISSING!
        blockShader.setFloat("time", currentFrame);
        world.draw(view, projection);

        // --- IMGUI HUD ---
        // Find nearest building to camera
        float nearestDist = 999999.0f;
        int nearestIdx = 0;
        for (int i = 0; i < (int)repos.size(); i++) {
            float d = glm::distance(camera.Position, repos[i].worldPos);
            if (d < nearestDist) {
                nearestDist = d;
                nearestIdx = i;
            }
        }
        if (nearestDist < 50.0f) {
            BuildingUI::render(repos[nearestIdx]);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    BuildingUI::shutdown();
    glfwTerminate();
    return 0;
}
