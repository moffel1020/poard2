cmake_minimum_required(VERSION 3.16)
project(poard2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(FETCHCONTENT_QUIET FALSE)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)

add_executable(poard2
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/window.cpp
    ${SRC_DIR}/shader.cpp
    ${SRC_DIR}/camera.cpp
    ${DEPS_DIR}/stb/stb_image.cpp
)

target_include_directories(poard2 PRIVATE
    ${SRC_DIR}
    ${DEPS_DIR}/stb/
)

target_compile_definitions(poard2 PRIVATE GLFW_INCLUDE_NONE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(glfw REQUIRED)

target_link_libraries(poard2 PRIVATE glfw glad glm::glm)
