set(ASSETS_SRC_DIR ${CMAKE_SOURCE_DIR}/assets)
set(ASSETS_DST_DIR ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB_RECURSE ASSETS_FILES "${ASSETS_SRC_DIR}/*")

# Copy assets to the build directory
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets.stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC_DIR} ${ASSETS_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/assets.stamp
    DEPENDS ${ASSETS_FILES}
    COMMENT "Copying assets..."
    VERBATIM
)

add_custom_target(Assets ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/assets.stamp
)