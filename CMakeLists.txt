cmake_minimum_required(VERSION 3.16)
project(poard2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(FETCHCONTENT_QUIET FALSE)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)

add_executable(poard2
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/window.cpp
    ${SRC_DIR}/shader.cpp
    ${SRC_DIR}/shader_program.cpp
    ${SRC_DIR}/camera.cpp
    ${SRC_DIR}/terrain_gen.cpp
    ${SRC_DIR}/imgui_wrapper.cpp
)

target_include_directories(poard2 PRIVATE
    ${SRC_DIR}
)

target_compile_definitions(poard2 PRIVATE GLFW_INCLUDE_NONE GLM_ENABLE_EXPERIMENTAL)

function(add_warnings target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /W3)
    endif()
endfunction()

add_warnings(poard2)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/deps.cmake)

target_link_libraries(poard2 PRIVATE glfw glad glm::glm stb imgui_glfw_ogl3)
