project(fcad)
link_libraries(Comctl32)
add_compile_options(/diagnostics:column,caret)

add_executable(fcad "main.cpp" "include/helpers.h" "include/action.h" "src/action.cpp" "include/fcad_events.h" "include/window_action_controller.h" "src/window_action_controller.cpp" "include/mode_controller.h" "src/mode_controller.cpp" "include/actions/create.h" "src/actions/create.cpp" "include/command.h" "include/command_controller.h" "src/command_controller.cpp" "include/parsing.h" "src/parsing.cpp" "include/commands.h" "src/commands/create_vertex.cpp" "include/geometry_controller.h" "include/camera_controller.h" "src/camera_controller.cpp" "src/geometry_controller.cpp" "src/commands/create_edge.cpp" "src/commands/create_face.cpp" "include/actions/misc.h" "src/actions/misc.cpp" "src/commands/focus.cpp" "src/commands/noop.cpp" "include/actions/delete.h" "src/actions/delete.cpp" "src/commands/delete_vertex.cpp" "src/commands/quit.cpp" "src/commands/delete_edge.cpp" "src/commands/delete_face.cpp")
add_custom_target(fcad_copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(fcad_copy_assets fcad)

target_include_directories(fcad PRIVATE include)
