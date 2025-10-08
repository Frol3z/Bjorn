cmake_minimum_required(VERSION 3.10)
project(Bjorn)

include(${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# -----------------------------
# Vulkan C++20 module
# -----------------------------
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_include_directories(VulkanCppModule
    PRIVATE
    "${Vulkan_INCLUDE_DIR}"
)

target_link_libraries(VulkanCppModule
    PUBLIC
    Vulkan::Vulkan
)

target_sources(VulkanCppModule
    PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS "${Vulkan_INCLUDE_DIR}"
    FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)

# -----------------------------
# Fetch dependencies
# -----------------------------
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# VMA
FetchContent_Declare(
    VMA
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)
FetchContent_MakeAvailable(VMA)

# -----------------------------
# Build executable
# -----------------------------
add_executable(Bjorn 
    "src/Main.cpp" 
    "src/Window.cpp" 
    "src/Application.cpp" 
    "src/Renderer.cpp" 
    "src/Swapchain.cpp" 
    "src/VmaUsage.cpp"
)

target_include_directories(Bjorn PRIVATE ${CMAKE_SOURCE_DIR}/include ${VMA_SOURCE_DIR}/include)

target_link_libraries(Bjorn
    PRIVATE
    Vulkan::cppm
    glfw
    glm::glm
)

# Shader dependencies
set (SHADER_SLANG_SOURCES ${CMAKE_SOURCE_DIR}/shaders/shader.slang)

add_slang_shader_target(Shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(Bjorn Shaders)

# Copy resources to the build folder
add_custom_command(
    TARGET Bjorn POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Bjorn>/shaders
)