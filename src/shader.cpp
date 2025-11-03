#include "shader.h"
#include "glad/gl.h"
#include <array>
#include <iostream>

constexpr size_t INFO_SIZE = 4096;

Shader::Shader(const std::string& vertSource, const std::string& fragSource) {
    id = glCreateProgram();

    const uint32_t vs = compileShader(GL_VERTEX_SHADER, vertSource);
    const uint32_t fs = compileShader(GL_FRAGMENT_SHADER, fragSource);
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    int succes;
    glGetProgramiv(id, GL_LINK_STATUS, &succes);
    if (!succes) {
        static std::array<char, INFO_SIZE> linkInfo{};
        glGetProgramInfoLog(id, linkInfo.size(), nullptr, linkInfo.data());
        std::cout << "ERROR: vertex and fragment shaders failed to link\n" << linkInfo.data() << std::endl;
    }

    glValidateProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

uint32_t Shader::compileShader(GLenum type, const std::string& source) {
    const char* src = source.c_str();
    const uint32_t id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        static std::array<char, INFO_SIZE> compileInfo{};
        glGetShaderInfoLog(id, compileInfo.size(), nullptr, compileInfo.data());
        std::cout << "ERROR: shader compilation of type " << type << " failed\n" << compileInfo.data() << std::endl;
    }

    return id;
}

void Shader::bind() const { glUseProgram(id); }

Shader::~Shader() { glDeleteProgram(id); }
