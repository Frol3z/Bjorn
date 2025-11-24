# Automatically find all .slang shader files in this folder
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "*.hlsl")

# Output directory inside the build folder
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Find DXC (HLSL) compiler
find_program(DXC_EXECUTABLE dxc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "dxc not found! Set VULKAN_SDK environment variable.")
endif()

# Find GLSL compiler
find_program(GLSLANG_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found! Set VULKAN_SDK environment variable.")
endif()

# Compile each shader individually

# Dear ImGui custom vertex shader (color fix)
set(IMGUI_VERT_SPV ${OUTPUT_DIR}/imgui_custom.vert.spv)
add_custom_command(
    OUTPUT ${IMGUI_VERT_SPV}
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/imgui_custom.vert.glsl
            -o ${IMGUI_VERT_SPV}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/imgui_custom.vert.glsl
    COMMENT "Compiling Dear ImGui custom vertex shader into ${IMGUI_VERT_SPV}.."
)

# HLSL shaders
set(COMPILED_SHADERS ${IMGUI_VERT_SPV})
foreach(SRC ${SHADER_SOURCES})
    get_filename_component(SRC_NAME ${SRC} NAME_WE)
    get_filename_component(SRC_EXT ${SRC} EXT)

    set(VERT_OUT ${OUTPUT_DIR}/${SRC_NAME}.vert.spv)
    set(FRAG_OUT ${OUTPUT_DIR}/${SRC_NAME}.frag.spv)

    if(SRC_EXT STREQUAL ".vert.hlsl")
        # Vertex shader
        add_custom_command(
            OUTPUT ${VERT_OUT}
            COMMAND ${DXC_EXECUTABLE} ${SRC}
                    -spirv
                    -T vs_6_0
                    -E main
                    -Fo ${VERT_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${SRC}
            COMMENT "Compiling vertex shader into ${VERT_OUT}.."
            VERBATIM
        )
    elseif(SRC_EXT STREQUAL ".frag.hlsl")
        # Fragment shader
        add_custom_command(
            OUTPUT ${FRAG_OUT}
            COMMAND ${DXC_EXECUTABLE} ${SRC}
                    -spirv
                    -T ps_6_0
                    -E main
                    -Fo ${FRAG_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${SRC}
            COMMENT "Compiling fragment shader into ${FRAG_OUT}.."
            VERBATIM
        )
    else()
        message(WARNING "Unknown shader extension: ${SRC}, skipping compilation") 
        continue() 
    endif()

    list(APPEND COMPILED_SHADERS ${VERT_OUT} ${FRAG_OUT})
endforeach()

# Custom target for all shaders
add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})