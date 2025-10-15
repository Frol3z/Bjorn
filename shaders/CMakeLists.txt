# Automatically find all .slang shader files in this folder
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "*.slang")

# Output directory inside the build folder
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Find Slang compiler
find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
if(NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR "slangc not found! Set VULKAN_SDK environment variable.")
endif()

# Compile each shader individually
set(COMPILED_SHADERS)
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