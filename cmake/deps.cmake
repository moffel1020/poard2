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
# set(GLM_ENABLE_SIMD_AVX2 ON)
FetchContent_Declare(
    glm
    GIT_REPOSITORY	https://github.com/g-truc/glm.git
    GIT_TAG 	1.0.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

# glad
find_package(glad REQUIRED)

# stb
find_package(stb REQUIRED)
