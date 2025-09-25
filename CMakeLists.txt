cmake_minimum_required(VERSION 3.16)
project(poard2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_executable(poard2
    ${SRC_DIR}/main.cpp
)

target_include_directories(poard2 PRIVATE
    ${SRC_DIR}
)