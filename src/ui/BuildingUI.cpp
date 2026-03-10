#include "BuildingUI.h"

void BuildingUI::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void BuildingUI::render(const RepoData& repo) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    
    ImGui::Begin(repo.name.c_str());
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Primary Language: %s", repo.language.c_str());
    ImGui::Separator();
    ImGui::TextWrapped("%s", repo.description.c_str());
    ImGui::Separator();
    ImGui::Text("Stars: %d", repo.stars);
    ImGui::Text("Forks: %d", repo.forks);
    ImGui::Text("Open Issues: %d", repo.openIssues);
    ImGui::Text("Size: %lld KB", repo.size);
    ImGui::Separator();
    ImGui::Text("Last Pushed: %s", repo.updatedAt.c_str());
    
    if (ImGui::Button("Close (ESC)")) {
        // Handle close
    }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BuildingUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
