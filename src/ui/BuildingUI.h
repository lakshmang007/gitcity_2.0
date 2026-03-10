#ifndef BUILDINGUI_H
#define BUILDINGUI_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "../github/RepoData.h"

class BuildingUI {
public:
    static void init(GLFWwindow* window);
    static void render(const RepoData& repo);
    static void shutdown();
};

#endif
