message("Fetching imgui...")

FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "ImGui source dir")