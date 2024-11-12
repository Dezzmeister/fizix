add_library(core STATIC
	src/lib.cpp
 "include/lib.h" "include/event.h")

target_include_directories(core PRIVATE include)