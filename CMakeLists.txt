cmake_minimum_required(VERSION 3.25)
project(VoxelGame)
cmake_policy(SET CMP0072 NEW)

#add_compile_definitions(RELEASE) <-- uncomment to remove logging at compile time

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(spdlog REQUIRED)
set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE SOURCE "src/*.cpp" "include/*.h" "include/*.hpp" "libs/*.cpp" "libs/*.c" "libs/*.h" "libs/*.hpp")

include_directories("include/" "libs/")
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL spdlog::spdlog_header_only)

# idfk cmake sucks. I'll leave this commented for now
#target_precompile_headers(${PROJECT_NAME} PUBLIC include/pch.h)
#set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
