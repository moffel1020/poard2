if (NOT glfw_FOUND)
    include(FetchContent)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_INSTALL "" OFF)

    FetchContent_Declare(
        GLFW
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(GLFW)
endif()
