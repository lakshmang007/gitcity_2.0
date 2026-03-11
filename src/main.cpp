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

// Camera
Camera camera(glm::vec3(60.0f, 80.0f, 60.0f));
Player player(camera);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// State
float flySpeed = 40.0f;
bool showLegend = true;

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
        flySpeed = std::max(5.0f, flySpeed - 0.3f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        flySpeed = std::min(200.0f, flySpeed + 0.3f);

    // Toggle legend
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) { showLegend = !showLegend; tabPressed = true; }
    } else { tabPressed = false; }
    
    if (camera.Position.y < 1.0f) camera.Position.y = 1.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Position.y -= yoffset * 8.0f;
    if (camera.Position.y < 1.0f) camera.Position.y = 1.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // --- INPUT ---
    std::string input;
    std::cout << "\n";
    std::cout << "  ================================================" << std::endl;
    std::cout << "       GitCity: Virtual Earth Explorer              " << std::endl;
    std::cout << "  ================================================" << std::endl;
    std::cout << "   Your GitHub profile as a fantasy world map!      " << std::endl;
    std::cout << "  ------------------------------------------------" << std::endl;
    std::cout << "\n";
    std::cout << "  Paste your GitHub profile link" << std::endl;
    std::cout << "  (e.g. https://github.com/octocat)" << std::endl;
    std::cout << "\n";
    std::cout << "  > " << std::flush;
    std::cin >> input;

    // Extract username from URL or use as-is
    std::string username = input;
    // Remove trailing slashes, .git suffix
    while (!username.empty() && username.back() == '/') username.pop_back();
    if (username.size() > 4 && username.substr(username.size() - 4) == ".git")
        username = username.substr(0, username.size() - 4);
    // Extract from github.com URL
    size_t pos = username.find("github.com/");
    if (pos != std::string::npos) {
        username = username.substr(pos + 11);
        size_t slash = username.find("/");
        if (slash != std::string::npos) username = username.substr(0, slash);
    }

    std::cout << "\n  Building world for: " << username << std::endl;
    std::cout << "\n";

    // --- FETCH ---
    std::cout << "  Fetching repositories..." << std::endl;
    std::vector<RepoData> repos = GitHubAPI::fetchUserRepos(username);
    if (repos.empty()) {
        std::cerr << "No repos found. Using demo data." << std::endl;
        RepoData d; d.name = "demo-repo"; d.language = "Unknown"; d.height = 8; d.width = 4;
        repos.push_back(d);
    }

    // --- INIT ---
    if (!glfwInit()) { std::cerr << "GLFW failed" << std::endl; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GitCity - Virtual Earth", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { return -1; }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    BuildingUI::init(window);

    Shader blockShader("shaders/block.vert", "shaders/block.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

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
    
    // Camera: start above the center of the map looking down
    float camX = 0, camZ = 0;
    for (auto& c : world.getCountries()) {
        camX += c.center.x;
        camZ += c.center.y;
    }
    camX /= std::max(1, (int)world.getCountries().size());
    camZ /= std::max(1, (int)world.getCountries().size());
    
    camera.Position = glm::vec3(camX, 150.0f, camZ + 50.0f);
    camera.Pitch = -50.0f;
    camera.Yaw = -90.0f;
    camera.updateCameraVectors();
    
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "WASD = Fly | Space/Shift = Up/Down | Mouse = Look" << std::endl;
    std::cout << "Scroll = Quick zoom | Q/E = Speed | TAB = Toggle legend" << std::endl;
    std::cout << "ESC = Exit" << std::endl;
    std::cout << "================\n" << std::endl;

    // --- MAIN LOOP ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.55f, 0.70f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.5f, 2000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // World
        blockShader.use();
        blockShader.setMat4("projection", projection);
        blockShader.setMat4("view", view);
        blockShader.setMat4("model", glm::mat4(1.0f));
        blockShader.setVec3("viewPos", camera.Position);
        blockShader.setFloat("time", currentFrame);
        world.draw(view, projection);

        // --- UI ---
        // Find nearest repo building
        float nearestDist = 999999.0f;
        int nearestIdx = -1;
        for (int i = 0; i < (int)repos.size(); i++) {
            float d = glm::distance(camera.Position, repos[i].worldPos);
            if (d < nearestDist) {
                nearestDist = d;
                nearestIdx = i;
            }
        }
        
        if (nearestDist < 30.0f && nearestIdx >= 0) {
            // Show repo details when close
            BuildingUI::render(repos[nearestIdx]);
        } else if (showLegend) {
            // Show the legend when not near a building
            BuildingUI::renderLegend(world.getCountries());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    BuildingUI::shutdown();
    glfwTerminate();
    return 0;
}
