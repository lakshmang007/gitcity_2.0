#include "BuildingUI.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void BuildingUI::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Dark theme with custom styling
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.Alpha = 0.92f;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void BuildingUI::render(const RepoData& repo) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Always);
    ImGui::Begin("Repository Info", nullptr, 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", repo.name.c_str());
    ImGui::Separator();
    
    if (!repo.description.empty())
        ImGui::TextWrapped("%s", repo.description.c_str());
    
    ImGui::Spacing();
    ImGui::Text("Language: %s", repo.language.c_str());
    ImGui::Text("Stars: %d  |  Forks: %d", repo.stars, repo.forks);
    ImGui::Text("Open Issues: %d", repo.openIssues);
    ImGui::Text("Size: %d KB", repo.size);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BuildingUI::renderLegend(const std::vector<CountryInfo>& countries) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Legend window - bottom right
    float windowW = 280.0f;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - windowW - 10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowW, 0), ImGuiCond_Always);
    ImGui::Begin("World Legend", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Programming Language Countries");
    ImGui::Separator();
    ImGui::Spacing();
    
    for (const auto& country : countries) {
        // Color swatch
        ImVec4 col(country.color.r, country.color.g, country.color.b, 1.0f);
        ImGui::ColorButton(("##" + country.language).c_str(), col, 
            ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker, ImVec2(16, 16));
        ImGui::SameLine();
        ImGui::Text("%s (%s) - %d repos", 
            country.fantasyName.c_str(), 
            country.language.c_str(), 
            country.repoCount);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "WASD=Fly Space/Shift=Up/Down");
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Scroll=Zoom Q/E=Speed");
    
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BuildingUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
