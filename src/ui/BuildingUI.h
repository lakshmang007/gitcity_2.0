#ifndef BUILDINGUI_H
#define BUILDINGUI_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "../github/RepoData.h"
#include "../world/WorldGenerator.h"
#include <vector>

class BuildingUI {
public:
    static void init(GLFWwindow* window);
    static void render(const RepoData& repo);
    static void renderLegend(const std::vector<CountryInfo>& countries);
    static void shutdown();
};

#endif
