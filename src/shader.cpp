#include "shader.h"
#include <array>
#include <iostream>

static std::array<char, 1024> compileInfo{};

Shader::Shader(const std::string& source, ShaderType type) {
    const char* src = source.c_str();
    id = glCreateShader(static_cast<GLenum>(type));
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id, compileInfo.size(), nullptr, compileInfo.data());
        std::cout << "ERROR: shader compilation of type " << static_cast<GLenum>(type) << " failed\n"
                  << compileInfo.data() << std::endl;
    }
}
