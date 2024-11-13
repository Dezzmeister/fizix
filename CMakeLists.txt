cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)

project(Fizix)

add_subdirectory(glfw)
add_subdirectory(glm)

add_definitions(-DGLFW_INCLUDE_NONE -DGLM_FORCE_QUAT_DATA_XYZW)
include_directories(${CMAKE_SOURCE_DIR}/core/include)
add_compile_options(/W4 /WX /fp:strict)
link_libraries(glfw glm opengl32 Gdiplus)

add_subdirectory(core)

link_libraries(core)

add_subdirectory(phong_demo)
