# Bjorn

Bjorn is a Vulkan renderer I'm currently working on. This README will be updated soon.

# Features

# Roadmap
## Short term
- model loading
- texture support
- basic lighting
## Long term
- basic UI
- physically based lighting support
- post processing support
- fully raytraced pipeline

# Getting Started
## Prerequisites
- CMake 4.0 or higher
- C++20
- Vulkan 1.4+ SDK
## Dependencies
- [GLFW 3.4](https://www.glfw.org/)
- [GLM 1.0.1](https://github.com/g-truc/glm)
- [Vulkan Memory Allocator 3.3.0](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
## Build instructions
### Windows
1. Clone this repository:
```bash
git clone https://github.com/Frol3z/Bjorn.git
cd Bjorn
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
5. Set `Bjorn` as the **startup project** in Visual Studio.
6. Build and run the project.

# Examples

# References