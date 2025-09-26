cmake_minimum_required(VERSION 3.25)
project(VoxelGame VERSION 0.5.0)
set(CMAKE_CXX_STANDARD 20)

# --- Core Library ---
add_subdirectory(../core ${CMAKE_BINARY_DIR}/core)

# --- Application ---
FILE(GLOB_RECURSE APP_SOURCE "src/*.cpp")
add_executable(${PROJECT_NAME} ${APP_SOURCE})

target_link_libraries(${PROJECT_NAME} PRIVATE VoxelGameCore)
target_include_directories(${PROJECT_NAME} PRIVATE include/)

# --- Testing ---
enable_testing()

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)
FetchContent_MakeAvailable(googletest)

FILE(GLOB_RECURSE APP_SOURCE_NO_MAIN "src/*.cpp")
list(REMOVE_ITEM APP_SOURCE_NO_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

FILE(GLOB_RECURSE TEST_SOURCE "tests/*.cpp")
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCE} ${APP_SOURCE_NO_MAIN})

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        gtest
        gtest_main
        pthread
        VoxelGameCore
)

target_include_directories(${PROJECT_NAME}_tests PRIVATE include/)

add_test(NAME RunTests COMMAND ${PROJECT_NAME}_tests)