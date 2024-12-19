cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)

project(Fizix)

add_subdirectory(glm)

add_definitions(-DGLM_FORCE_QUAT_DATA_XYZW -DGLM_ENABLE_EXPERIMENTAL -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
include_directories(${CMAKE_SOURCE_DIR}/core/include)
add_compile_options(/W4 /WX /fp:strict /await:strict)
link_libraries(glm opengl32 Gdiplus)

add_subdirectory(core)

link_libraries(core)

add_subdirectory(phong_demo)
add_subdirectory(soft_physics_demo)
add_subdirectory(fcad)
add_subdirectory(tests)
