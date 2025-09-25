if (NOT glm_FOUND)
    include(FetchContent)

    set(GLM_ENABLE_CXX_17 ON)
    # set(GLM_ENABLE_SIMD_AVX2 ON)

    FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glm)
endif()
