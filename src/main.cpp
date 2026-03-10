#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "render/Shader.h"
#include "player/Camera.h"
#include "player/Player.h"
#include "github/GitHubAPI.h"
#include "world/WorldGenerator.h"
#include "ui/BuildingUI.h"
#include "render/TextRenderer.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera & Player
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
Player player(camera);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isGlobeView = true;
float altitude = 400.0f;
bool isDiving = false;
glm::vec3 diveTarget;
bool showHUD = true;

float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        player.handleInput(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        player.handleInput(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.handleInput(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.handleInput(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player.jump();
        
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) {
            showHUD = !showHUD;
            tabPressed = true;
        }
    } else {
        tabPressed = false;
    }

    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (!mPressed) {
            isGlobeView = !isGlobeView;
            if (isGlobeView) {
                altitude = 150.0f; // Snap to high altitude
                camera.Pitch = -89.0f; // Look straight down
            } else {
                altitude = 2.0f; // Back to ground
                camera.Pitch = 0.0f;
            }
            mPressed = true;
        }
    } else {
        mPressed = false;
    }
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && isGlobeView && !isDiving) {
        // Diving: in a more complex setup we'd raycast, but here we'll dive toward current look point on ground
        diveTarget = camera.Position + camera.Front * (camera.Position.y + 10.0f); 
        diveTarget.y = 2.0f; 
        isDiving = true;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (isGlobeView) {
        altitude -= yoffset * 5.0f; // Zoom in/out altitude
        if (altitude < 2.0f) altitude = 2.0f;
        if (altitude > 500.0f) altitude = 500.0f;
    } else {
        camera.ProcessMouseScroll(yoffset);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    std::string input;
    std::cout << "Enter GitHub username or profile link: " << std::flush;
    std::cin >> input;

    std::string username = input;
    // Check if it's a URL (e.g., https://github.com/octocat)
    size_t pos = input.find("github.com/");
    if (pos != std::string::npos) {
        username = input.substr(pos + 11);
        // Remove trailing slashes
        size_t slash = username.find("/");
        if (slash != std::string::npos) {
            username = username.substr(0, slash);
        }
    }

    std::cout << "Starting in Virtual Earth mode. Use WASD to fly, M to toggle Global/City view." << std::endl;
    isGlobeView = true;
    camera.Position = glm::vec3(0.0f, 400.0f, 0.0f); // Look down from above
    camera.Pitch = -89.9f; 
    camera.Yaw = -90.0f;
    camera.updateCameraVectors();

    std::cout << "Fetching repositories for " << username << "..." << std::endl;
    std::vector<RepoData> repos = GitHubAPI::fetchUserRepos(username);
    if (repos.empty()) {
        std::cout << "No repositories found or error fetching data." << std::endl;
        RepoData dummy; dummy.name = "No repos found"; dummy.language = "Unknown";
        repos.push_back(dummy);
    }
    std::cout << "Found " << repos.size() << " repositories." << std::endl;

    std::cout << "Initializing GLFW..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    std::cout << "GLFW initialized. Creating window..." << std::endl;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GitCity", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "Window created. Setting up context..." << std::endl;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "GLAD initialized successfully." << std::endl;

    glEnable(GL_DEPTH_TEST);
    BuildingUI::init(window);

    TextRenderer textRenderer(SCR_WIDTH, SCR_HEIGHT);
    textRenderer.load("C:\\Windows\\Fonts\\arial.ttf", 48); 

    Shader blockShader("shaders/block.vert", "shaders/block.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    WorldGenerator world;
    world.generateWorld(repos);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        if (isDiving) {
            camera.Position = glm::mix(camera.Position, diveTarget, 0.05f); // Smooth dive
            if (glm::distance(camera.Position, diveTarget) < 1.0f) {
                isDiving = false;
                isGlobeView = false;
            }
        } else if (isGlobeView) {
            camera.Position.y = altitude;
        } else {
            player.update(deltaTime);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // Blocks
        blockShader.use();
        blockShader.setMat4("projection", projection);
        blockShader.setMat4("view", view);
        blockShader.setMat4("model", glm::mat4(1.0f));
        world.draw(view, projection);

        // Labels rendering pass
        for (const auto& label : world.getLabels()) {
            float dist = glm::distance(camera.Position, label.pos);
            
            // "Land of X" labels are visible from far away
            if (label.text.find("Land of") != std::string::npos) {
                if (dist < 500.0f) {
                    textRenderer.renderText3D(label.text, label.pos, 0.2f, glm::vec3(1.0f, 0.8f, 0.0f), view, projection);
                }
            } 
            // Building labels are only visible when zoomed in
            else if (dist < 40.0f) {
                textRenderer.renderText3D(label.text, label.pos, 0.05f, glm::vec3(1.0f), view, projection);
            }
        }

        // UI (topmost repo or nearest?)
        if (showHUD && !repos.empty()) {
            // Simple logic: show UI for nearest building if we are in CITY view
            if (!isGlobeView) {
                 BuildingUI::render(repos[0]); 
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    BuildingUI::shutdown();
    glfwTerminate();
    return 0;
}
