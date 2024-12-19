project(fcad)
link_libraries(Comctl32)
add_compile_options(/diagnostics:column,caret)

add_executable(fcad "main.cpp" "include/helpers.h" "include/action.h" "src/action.cpp" "include/fcad_events.h" "include/window_action_controller.h" "src/window_action_controller.cpp" "include/mode_controller.h" "src/mode_controller.cpp" "include/actions/create.h" "src/actions/create.cpp" "include/command.h" "include/command_controller.h" "src/command_controller.cpp" "include/parsing.h" "src/parsing.cpp" "include/commands.h" "src/commands/create_vertex.cpp" "include/geometry_controller.h")
add_custom_target(fcad_copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(fcad_copy_assets fcad)

target_include_directories(fcad PRIVATE include)
