# Felina

Felina is a Vulkan renderer I'm currently working on.

# Features

# Roadmap
## Short term
- deferred rendering
- material system
- basic lighting
## Long term
- PBR materials
- cascaded shadow maps
- hybrid GI

# Getting Started
## Prerequisites
- CMake 4.0 or higher
- C++20
- Vulkan 1.4+ SDK
## Dependencies
- [GLFW 3.4](https://www.glfw.org/)
- [GLM 1.0.1](https://github.com/g-truc/glm)
- [Vulkan Memory Allocator 3.3.0](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [Dear ImGui 1.92.4 (docking)](https://github.com/ocornut/imgui)
## Build instructions
### Windows
1. Clone this repository:
```bash
git clone https://github.com/Frol3z/Felina.git
cd Felina
```
2. Create and enter the build directory:
```bash
mkdir build
cd build
```
3. Generate project files using CMake:
```bash
cmake ..
```
4. Open the generated **solution** inside the build folder.
5. Set `Felina` as the **startup project** in Visual Studio.
6. Build and run the project.

# Class diagram
![Felina](https://github.com/user-attachments/assets/2db46a15-cc66-4667-97b0-2f1dc718a246)

# Examples

# References
