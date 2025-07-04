cmake_minimum_required(VERSION 3.25)
project(VoxelGame VERSION 0.2.0)
cmake_policy(SET CMP0072 NEW)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

# uncomment to remove logging at compile time
# add_compile_definitions(NOLOG)

# uncomment to remove performance profiling at compile time
add_compile_definitions(NOPROFILE)

include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

# Fetch Custom Libraries
FetchContent_Declare(
        cstmlib
        GIT_REPOSITORY https://github.com/DeletedMyEmail/CustomLibs.git
        GIT_TAG cpp
)
FetchContent_MakeAvailable(cstmlib)

# Fetch FastNoiseLite
FetchContent_Declare(
        fastnoise
        GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
        GIT_TAG master
)
FetchContent_MakeAvailable(fastnoise)

# Fetch ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.7  # Adjust version/tag as needed
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

FILE(GLOB_RECURSE SOURCE "src/*.cpp" "libs/*.cpp" "libs/*.c")
include_directories("include/" "libs/")

add_executable(${PROJECT_NAME} ${SOURCE})
# ImGui include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${fastnoise_SOURCE_DIR}/Cpp
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

# Add ImGui sources to the executable
target_sources(${PROJECT_NAME} PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


# Link libraries
target_link_libraries(${PROJECT_NAME}
        glfw
        OpenGL::GL
        cstmlib
)

# Add testing ---------------------------------------------------------------

enable_testing()

# Fetch Google Test
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
FILE(GLOB_RECURSE TEST_SOURCE "tests/*.cpp" "src/*.cpp" "libs/*.cpp" "libs/*.c")
list(REMOVE_ITEM TEST_SOURCE "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(${PROJECT_NAME}_tests ${TEST_SOURCE})

target_link_libraries(${PROJECT_NAME}_tests
        gtest
        gtest_main
        pthread
        glfw
        OpenGL::GL
        spdlog::spdlog_header_only
)

# Add test command
add_test(NAME RunTests COMMAND ${PROJECT_NAME}_tests)