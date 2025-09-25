if (NOT glad_FOUND)
    add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glad glad)
    set(glad_FOUND 1)
endif()
