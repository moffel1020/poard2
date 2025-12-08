#include "shader_program.h"
#include "shader.h"
#include <array>
#include <glad/gl.h>
#include <iostream>

static std::array<char, 1024> linkInfo{};

ShaderProgram::ShaderProgram(std::initializer_list<Shader> shaders) {
    id = glCreateProgram();

    for (const auto& s : shaders) {
        glAttachShader(id, s.handle());
    }

    glLinkProgram(id);

    int success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id, linkInfo.size(), nullptr, linkInfo.data());
        std::cout << "ERROR: shaders failed to link\n" << linkInfo.data() << std::endl;
        // TODO: throw error or something?
    }

    glValidateProgram(id);
}
