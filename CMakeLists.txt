cmake_minimum_required(VERSION 4.0)

project(
    Felina
    LANGUAGES CXX
    VERSION 1.0
)

# Force C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Dependencies first

# Search for installed Vulkan SDK
find_package(Vulkan REQUIRED)

# Vulkan C++20 module setup
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_compile_definitions(VulkanCppModule PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
target_include_directories(VulkanCppModule PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)
target_sources(VulkanCppModule PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS "${Vulkan_INCLUDE_DIR}"
    FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)
add_subdirectory(dependencies)

# Then source files and main target
add_subdirectory(src)

# Then additional resources
add_subdirectory(shaders)
add_dependencies(Felina Shaders)


