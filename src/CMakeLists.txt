# Source files
add_executable(Bjorn
    "Main.cpp" 
    "Window.cpp" 
    "Application.cpp" 
    "Renderer.cpp" 
    "Swapchain.cpp" 
    "VmaUsage.cpp"
    "Buffer.cpp"
)

# Includes
target_include_directories(Bjorn PRIVATE ${VMA_SOURCE_DIR}/include)

# Libraries
target_link_libraries(Bjorn PRIVATE
    Vulkan::cppm
    glfw
    glm::glm
)