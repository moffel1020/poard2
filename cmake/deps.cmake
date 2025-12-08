cmake_minimum_required(VERSION 3.16)
include(FetchContent)

# GLFW
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_INSTALL "" OFF)
FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(GLFW)

# glm
set(GLM_ENABLE_CXX_17 ON)
set(GLM_ENABLE_SIMD_AVX2 ON)
FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/releases/download/1.0.2/glm-1.0.2.zip
)
FetchContent_MakeAvailable(glm)

# glad
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glad glad)

# stb
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/stb stb)

# imgui
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.5.tar.gz
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
add_library(imgui_glfw_ogl3
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui_glfw_ogl3 PUBLIC 
    ${glfw_SOURCE_DIR}/include
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
)
