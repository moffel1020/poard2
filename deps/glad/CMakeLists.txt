cmake_minimum_required(VERSION 3.16)
project(glad)

add_library(glad 
    "src/gl.c"
    "include/glad/gl.h"
)

target_include_directories(glad SYSTEM PUBLIC include/)
