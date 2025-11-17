# Automatically find all .slang shader files in this folder
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "*.slang")

# Output directory inside the build folder
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Find Slang compiler
find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
if(NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR "slangc not found! Set VULKAN_SDK environment variable.")
endif()

# Find GLSL compiler
find_program(GLSLANG_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found! Set VULKAN_SDK environment variable.")
endif()

# Compile each shader individually

# Dear ImGui custom vertex shader (color fix)
set(IMGUI_VERT_SPV ${OUTPUT_DIR}/imgui_custom.spv)
add_custom_command(
    OUTPUT ${OUTPUT_DIR}/imgui_custom.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/imgui_custom.vert
            -o ${IMGUI_VERT_SPV}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/imgui_custom.vert
    COMMENT "Compiling Dear ImGui custom vertex shader"
)

# My shaders
set(COMPILED_SHADERS ${IMGUI_VERT_SPV})
foreach(SRC ${SHADER_SOURCES})
    get_filename_component(SRC_NAME ${SRC} NAME_WE)
    set(OUT_FILE ${OUTPUT_DIR}/${SRC_NAME}.spv)

    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${SLANGC_EXECUTABLE} ${SRC} 
                -target spirv 
                -profile spirv_1_4 
                -emit-spirv-directly 
                -fvk-use-entrypoint-name -entry vertMain -entry fragMain
                -o ${OUT_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${SRC}
        COMMENT "Compiling shader: ${SRC_NAME}"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUT_FILE})
endforeach()

# Custom target for all shaders
add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})