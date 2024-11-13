project(soft_physics_demo)

add_executable(soft_physics_demo "main.cpp" "include/connector_spawn_tool.h" "include/constants.h" "include/custom_events.h" "include/gui.h" "include/object_world.h" "include/particle_collision_constraint_generator.h" "include/particle_spawn_tool.h" "include/particle_utils.h" "include/pointer_tool.h" "include/raycast.h" "include/tools.h" "src/connector_spawn_tool.cpp" "src/constants.cpp" "src/gui.cpp" "src/particle_spawn_tool.cpp" "src/particle_utils.cpp" "src/pointer_tool.cpp" "src/raycast.cpp" "src/tools.cpp")
add_custom_target(soft_physics_demo_copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(soft_physics_demo_copy_assets soft_physics_demo)

target_include_directories(soft_physics_demo PRIVATE include)
