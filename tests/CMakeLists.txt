project(tests)

add_executable(tests "main.cpp" "src/base64_test.cpp" "src/bvh_test.cpp" "src/collision_test.cpp" "src/ipaddr_test.cpp" "src/json_test.cpp" "src/matchers.cpp" "src/setup.cpp" "src/uri_test.cpp")
add_custom_target(tests_copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(tests_copy_assets tests)

target_include_directories(tests PRIVATE include)
