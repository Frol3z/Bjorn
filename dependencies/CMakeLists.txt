include(FetchContent)

# glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4
)
add_subdirectory(glfw)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # 1.0.1
)
add_subdirectory(glm)

# vma
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG 1d8f600fd424278486eade7ed3e877c99f0846b1 # v3.3.0
)
add_subdirectory(vma)

# imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG e7d2d636affeeac1fc7175fb13472ca02d1bc0d1 # v1.92.4-docking
)
add_subdirectory(imgui)

# tinygltf
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG 81bd50c1062fdb956e878efa2a9234b2b9ec91ec # v2.9.7
)
add_subdirectory(tinygltf)