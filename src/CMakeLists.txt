# Source files
file(GLOB ALL_SRC_FILES "*.cpp" "*.hpp")
add_executable(Bjorn ${ALL_SRC_FILES})

# Includes
target_include_directories(Bjorn PRIVATE ${VMA_SOURCE_DIR}/include)

# Libraries
target_link_libraries(Bjorn PRIVATE
    Vulkan::cppm
    glfw
    glm::glm
)