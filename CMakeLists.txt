cmake_minimum_required(VERSION 3.25)
project(VoxelGame VERSION 0.2.0)
cmake_policy(SET CMP0072 NEW)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

# uncomment to remove logging at compile time
#add_compile_definitions(NOLOG)

# uncomment to remove performance profiling at compile time
add_compile_definitions(NOPROFILE)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git

)
FetchContent_MakeAvailable(glfw)

include(FetchContent)
FetchContent_Declare(
        cstmlib
        GIT_REPOSITORY https://github.com/DeletedMyEmail/CustomLibs.git
        GIT_TAG cpp
)
FetchContent_MakeAvailable(cstmlib)

include(FetchContent)
FetchContent_Declare(
        fastnoise
        GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
        GIT_TAG master
)
FetchContent_MakeAvailable(fastnoise)

FIND_PACKAGE(OpenGL REQUIRED)

FILE(GLOB_RECURSE SOURCE "src/*.cpp" "libs/*.cpp" "libs/*.c")
include_directories("include/" "libs/")

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL cstmlib)
target_include_directories(${PROJECT_NAME} PRIVATE ${fastnoise_SOURCE_DIR}/Cpp)

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
