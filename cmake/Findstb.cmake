if (NOT stb_FOUND)
    add_subdirectory(${PROJECT_SOURCE_DIR}/deps/stb stb)
    set(stb_FOUND 1)
endif()
