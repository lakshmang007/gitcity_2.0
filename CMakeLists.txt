cmake_minimum_required(VERSION 3.17)
project(gitcity)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use system packages from MSYS2
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Freetype REQUIRED)

# GLM is header-only in MSYS2
find_path(GLM_INCLUDE_DIR glm/glm.hpp)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# Dear ImGui - fetch since not in MSYS2
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC 
        ${imgui_SOURCE_DIR} 
        ${imgui_SOURCE_DIR}/backends
        ${CMAKE_SOURCE_DIR}/src/render/gladgen/include
    )
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
endif()

# stb_image (header only) - download if needed
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb_image.h")
    file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "${CMAKE_SOURCE_DIR}/external/stb_image.h")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/render/Shader.cpp
    src/render/TextRenderer.cpp
    src/render/stb_image_impl.cpp
    src/render/gladgen/src/gl.c
    src/github/GitHubAPI.cpp
    src/world/Chunk.cpp
    src/world/WorldGenerator.cpp
    src/ui/BuildingUI.cpp
)

add_executable(gitcity ${SOURCES})

target_include_directories(gitcity PRIVATE 
    src 
    src/render/gladgen/include
    ${CMAKE_SOURCE_DIR}/external
    ${GLM_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(gitcity PRIVATE 
    glfw
    OpenGL::GL
    nlohmann_json::nlohmann_json 
    imgui 
    CURL::libcurl
    Freetype::Freetype
)

target_link_options(gitcity PRIVATE -static-libstdc++ -static-libgcc)

# Copy shaders and textures to build directory
add_custom_command(TARGET gitcity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:gitcity>/shaders
)
add_custom_command(TARGET gitcity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:gitcity>/textures
)
