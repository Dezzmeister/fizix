project(fcad)

add_executable(fcad "main.cpp")
add_custom_target(fcad_copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(fcad_copy_assets fcad)

target_include_directories(fcad PRIVATE include)
