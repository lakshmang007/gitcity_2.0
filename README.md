# GitCity - Virtual Earth GitHub Visualizer

GitCity transforms your GitHub profile into a **Virtual Earth** where programming languages are **Countries** and repositories are **Cities** within them. 

## ✨ Features
- **🌍 Virtual Earth Structure**:
  - **Languages = Countries**: Each programming language is its own island/continent in a vast voxel ocean.
  - **Repositories = Cities/Buildings**: Each repo is a skyscraper with unique dimensions:
    - **Height**: Based on the repository's code size (logarithmic).
    - **Width**: Based on stars + activity (square root).
    - **Lit Windows**: Buildings feature emissive windows (yellow voxels) representing development activity.
    - **Glowing Beacons**: Repos with open issues have glowing glass beacons on their roofs.
- **🚀 Dynamic Exploration**:
  - **Orbit View**: Start in space and see your "Language Countries" from high altitude.
  - **Cinematic Dive**: Left-click any country to perform a smooth, cinematic dive from space into that city.
  - **M-Key Overview**: Instantly toggle between the global map overview and local city exploration.
- **🏷️ Floating Labels**: Repo names fade into view as you approach buildings, with high-altitude headers for each country.
- **📊 Stats HUD**: Press **TAB** to toggle a detailed statistics panel for the nearest repository.

## Tech Stack
- C++17
- OpenGL 3.3 Core Profile
- GLFW, GLAD, GLM
- libcurl & nlohmann/json
- Dear ImGui
- FreeType 2

## Building the Project

### Prerequisites
- CMake 3.17+
- A C++17 compatible compiler (Visual Studio 2022 recommended on Windows)
- Internet connection (for FetchContent dependencies)

### Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be generated in the `Release` folder. Shaders and textures are automatically copied to the build directory.

## Controls
- **W/A/S/D**: Move
- **Space**: Jump
- **Mouse**: Look around
- **ESC**: Exit application
