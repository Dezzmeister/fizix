project(phong_demo)

add_executable(phong_demo "main.cpp")
add_custom_target(copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(copy_assets phong_demo)
