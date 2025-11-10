#pragma once
#include <cstdint>
#include <glad/gl.h>
#include <string>
#include <utility>

enum class ShaderType : GLenum {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Compute = GL_COMPUTE_SHADER,
};

class Shader {
public:
    Shader(const std::string& source, ShaderType type);

    Shader(const Shader& other) = delete;
    Shader& operator=(const Shader& other) = delete;

    Shader(Shader&& other) noexcept : id(std::exchange(other.id, 0)) {}

    Shader& operator=(Shader&& other) noexcept {
        id = std::exchange(other.id, 0);
        return *this;
    }

    uint32_t getId() const { return id; }

    ~Shader() {
        if (id != 0) {
            glDeleteShader(id);
        }
    }

private:
    uint32_t id;
};
