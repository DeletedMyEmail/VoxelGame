cmake_minimum_required(VERSION 3.25)
project(VoxelGameCore VERSION 0.5.0)
set(CMAKE_CXX_STANDARD 20)

# --- Dependencies ---
include(FetchContent)
find_package(OpenGL REQUIRED)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(cstmlib GIT_REPOSITORY https://github.com/DeletedMyEmail/CustomLibs.git GIT_TAG cpp)
FetchContent_MakeAvailable(cstmlib)

FetchContent_Declare(fastnoise GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git GIT_TAG master)
FetchContent_MakeAvailable(fastnoise)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.89.9)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(implot GIT_REPOSITORY https://github.com/epezent/implot.git GIT_TAG v0.16)
FetchContent_MakeAvailable(implot)

FetchContent_Declare(SQLiteCpp GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git GIT_TAG 3.3.3)
FetchContent_MakeAvailable(SQLiteCpp)

FetchContent_Declare(libconfig GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git GIT_TAG v1.8.1)
FetchContent_MakeAvailable(libconfig)

# --- Library Target ---
add_library(${PROJECT_NAME} STATIC)

# --- Sources ---
FILE(GLOB_RECURSE CORE_SOURCES "src/*.cpp" "libs/*.cpp" "libs/*.c")
target_sources(${PROJECT_NAME} PRIVATE
        ${CORE_SOURCES}
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
        ${libconfig_SOURCE_DIR}/lib
)

# --- Includes ---
target_include_directories(${PROJECT_NAME} PUBLIC
        include/
        libs/
        ${fastnoise_SOURCE_DIR}/Cpp
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
)

# --- Linking ---
target_link_libraries(${PROJECT_NAME} PUBLIC
        glfw
        OpenGL::GL
        cstmlib
        SQLiteCpp
        config++
)

# --- Compile Definitions ---
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJECT_NAME="${PROJECT_NAME}")