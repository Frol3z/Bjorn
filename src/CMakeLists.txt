# Source files
file(GLOB ALL_SRC_FILES "*.cpp" "*.hpp")
add_executable(Bjorn ${ALL_SRC_FILES})

# GLM defines specified here to avoid forgetting to define them each time a glm include is needed
add_compile_definitions(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Includes
target_include_directories(Bjorn PRIVATE ${VMA_SOURCE_DIR}/include)

# Libraries
target_link_libraries(Bjorn PRIVATE
    Vulkan::cppm
    glfw
    glm::glm
)