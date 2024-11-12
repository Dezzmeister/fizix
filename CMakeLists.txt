cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

project(Fizix)

add_subdirectory(core)

add_executable(Fizix main.cpp)

target_include_directories(Fizix PRIVATE core/include)
target_link_libraries(Fizix core)