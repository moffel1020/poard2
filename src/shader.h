#pragma once
#include <string>

class Shader {
public:
    Shader(const std::string& vertSource, const std::string fragSource);
    ~Shader();
    void bind() const;

    uint32_t getId() const { return id; };

private:
    static uint32_t compileShader(uint32_t type, const std::string& source);
    uint32_t id;
};
