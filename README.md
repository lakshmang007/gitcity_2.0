# GitCity - GitHub Voxel World Visualizer

Explore your GitHub profile as a 3D voxel city!

## Features
- **Biomes by Language**: Each programming language is assigned a unique biome.
- **Repository Buildings**:
  - **Height**: Based on repository size (logarithmic).
  - **Width**: Based on stars (square root).
  - **Beacons**: Buildings glow if they have open issues.
- **First-Person Exploration**: WASD to walk, Space to jump, mouse to look.
- **Interactive UI**: View repository details via an in-game HUD (powered by ImGui).
- **Infinite Potential**: Chunk-based voxel engine for future expansion.

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
