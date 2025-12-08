#pragma once
#include "shader.h"

class ShaderProgram {
public:
    ShaderProgram(std::initializer_list<Shader> shaders);

    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept : id(std::exchange(other.id, 0)) {}

    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        id = std::exchange(other.id, 0);
        return *this;
    }

    void bind() const { glUseProgram(id); }

    uint32_t handle() const { return id; };

    ~ShaderProgram() {
        if (id != 0) {
            glDeleteProgram(id);
        }
    }

private:
    uint32_t id;
};
