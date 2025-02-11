cmake_minimum_required(VERSION 3.25)
project(VoxelGame)
cmake_policy(SET CMP0072 NEW)

add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

# uncomment to remove logging at compile time
#add_compile_definitions(NOLOG)

# uncomment to remove performance profiling at compile time
#add_compile_definitions(NOPROFILE)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(spdlog REQUIRED)
set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE SOURCE "src/*.cpp" "libs/*.cpp" "libs/*.c")

include_directories("include/" "libs/")
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL spdlog::spdlog_header_only)

# Add testing ---------------------------------------------------------------

enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
FILE(GLOB_RECURSE TEST_SOURCE "tests/*.cpp" "src/*.cpp" "libs/*.cpp" "libs/*.c")
list(REMOVE_ITEM TEST_SOURCE "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(${PROJECT_NAME}_tests ${TEST_SOURCE})

target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main pthread glfw OpenGL::GL spdlog::spdlog_header_only)

# Add test command
add_test(NAME RunTests COMMAND ${PROJECT_NAME}_tests)