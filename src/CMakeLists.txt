# Source files
file(GLOB FELINA_SRC_FILES "*.cpp" "*.hpp")

# Dear ImGui integration
file(GLOB IMGUI_ROOT_SRC_FILES "${IMGUI_SOURCE_DIR}/*.cpp" "${IMGUI_SOURCE_DIR}/*.h")
set(IMGUI_BACKENDS_SRC_FILES 
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)
file(GLOB IMGUI_MISC_SRC_FILES "${IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.*")

# ImGui debug stuff for VS (remove if useless)
if(MSVC)
    set(IMGUI_DEBUG_FILES
        "${IMGUI_SOURCE_DIR}/misc/debuggers/imgui.natvis"
        "${IMGUI_SOURCE_DIR}/misc/debuggers/imgui.natstepfilter"
    )
endif()

# Target definition
add_executable(Felina
    ${FELINA_SRC_FILES} 
    ${IMGUI_ROOT_SRC_FILES} ${IMGUI_BACKENDS_SRC_FILES} ${IMGUI_MISC_SRC_FILES}
)
set_target_properties(Felina PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(MSVC)
    target_sources(Felina PRIVATE ${IMGUI_DEBUG_FILES})
endif()

# GLM defines specified here to avoid forgetting to define them each time a glm include is needed
add_compile_definitions(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Includes
target_include_directories(Felina PRIVATE ${VMA_SOURCE_DIR}/include ${IMGUI_SOURCE_DIR})

# Libraries
target_link_libraries(Felina PRIVATE
    Vulkan::cppm
    glfw
    glm::glm
)