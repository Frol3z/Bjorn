# Felina

![Renderer](rendering.png)

Felina is a Vulkan renderer written in C++.

Initially started as a simple project with the goal of learning the Vulkan API, 
it has since evolved into a small (relatively speaking) framework which is actively being extended. 
Its purpose is to provide a space for experimenting on topics such as real time rendering and game development.

# Features
- simple user interface
- full deferred rendering pipeline
- material system using Blinn-Phong lighting model
# Roadmap
## Short term
- texture support
- multiple lights
- improved UX
- glTF loading
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

# Architecture
![Diagram](diagram.png)

## GBuffer structure
| Attachment # | R | G | B | A |
|---|---|---|---|---|
| 0 | Albedo.R | Albedo.G | Albedo.B | Unused |
| 1 | Specular.R | Specular.G | Specular.B | Unused|
| 2 | Ambient Coeff. | Diffuse Coeff. | Specular Coeff. | Unused |
| 3 | Normal.X | Normal.Y | Normal.Z | Unused |
| 4 | Depth | Depth | Depth | Depth |
# Examples

# References

## General
https://docs.vulkan.org/tutorial/latest/00_Introduction.html
https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html
https://www.learncpp.com/
https://www.realtimerendering.com/
https://docs.vulkan.org/guide/latest/hlsl.html

## On specific topics
https://developer.nvidia.com/vulkan-memory-management
https://wallisc.github.io/rendering/2021/04/18/Fullscreen-Pass.html
