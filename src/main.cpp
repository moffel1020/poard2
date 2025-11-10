#include "camera.h"
#include "input.h"
#include "noise.h"
#include "shader.h"
#include "shader_program.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <fstream>
#include <functional>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <stb_image.h>

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;
};

static void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam) {
    (void)length;
    (void)userParam;

    // clang-format off
    const auto srcStr = [source]() {
        switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "";
        }
    }();

    const auto typeStr = [type]() {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        default: return "";
        }
    }();

    const auto severityStr = [severity]() {
        switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        default: return "";
        }
    }();
    // clang-format on
    std::cout << srcStr << ", " << typeStr << ", " << severityStr << ", " << id << ": " << message << '\n';
}

std::vector<Vertex> genHeightmap(int32_t w, int32_t h) {
    constexpr int32_t gridSize = 200;
    constexpr int32_t octaves = 12;
    constexpr float lacunarity = 2;
    constexpr float gain = 0.5;

    std::vector<Vertex> heightmap;
    heightmap.reserve(w * h);

    for (int32_t z = 0; z < h; z++) {
        for (int32_t x = 0; x < w; x++) {
            float val = 0;
            float freq = 1;
            float amp = 1;

            for (int32_t i = 0; i < octaves; i++) {
                val += Noise::perlin(x * freq / gridSize, z * freq / gridSize) * amp;
                freq *= lacunarity;
                amp *= gain;
            }

            val = std::clamp(val * 1.2f, -1.0f, 1.0f);

            float height = (((val + 1.0f) * 0.5f));
            heightmap.emplace_back(Vertex{{x, height, z}, {0.0f, 0.0f}});
        }
    }

    return heightmap;
}

std::vector<uint32_t> genHeightIndices(int32_t w, int32_t h) {
    std::vector<uint32_t> indices;
    indices.reserve((w - 1) * (h - 1) * 2 * 3);

    for (int32_t j = 0; j < h - 1; j++) {
        for (int32_t i = 0; i < w - 1; i++) {
            int32_t tl = i + j * w;
            int32_t tr = i + 1 + j * w;
            int32_t bl = i + (j + 1) * w;
            int32_t br = i + 1 + (j + 1) * w;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(bl);
            indices.push_back(br);
            indices.push_back(tr);
        }
    }

    return indices;
}

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    Window window(1280, 720, "poard2");

    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        throw std::runtime_error("failed to initialize glad");
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&debugCallback, nullptr);

    const auto [w, h] = window.size();
    glViewport(0, 0, w, h);

    glfwSetFramebufferSizeCallback(
        window.handle(), [](GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); });

    const auto readFile = [](const char* path) {
        std::ifstream file(path);
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        std::stringstream sstr;
        file >> sstr.rdbuf();
        return sstr.str();
    };

    const std::string vertSrc = readFile("res/shaders/shader.vert");
    const std::string fragSrc = readFile("res/shaders/shader.frag");
    const ShaderProgram program({
        Shader(vertSrc, ShaderType::Vertex),
        Shader(fragSrc, ShaderType::Fragment),
    });

    constexpr int32_t hWidth = 1024;
    constexpr int32_t hHeight = 1024;
    const auto vertices = genHeightmap(hWidth, hHeight);
    const auto indices = genHeightIndices(hWidth, hHeight);

    uint32_t vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    uint32_t ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    uint32_t vao;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(vao, ebo);

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);

    // uint32_t texture;
    // glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    // glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    // glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // int width, height, numChannels;
    // uint8_t* data = stbi_load("res/textures/container.jpg", &width, &height, &numChannels, 0);
    // if (!data) {
    //     throw std::runtime_error("failed to load image from disk");
    // }

    // glTextureStorage2D(texture, 1, GL_RGB8, width, height);
    // glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    // glGenerateTextureMipmap(texture);

    // stbi_set_flip_vertically_on_load(true);
    // stbi_image_free(data);

    glm::mat4 model(1.0f);
    // model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
    Camera cam(static_cast<float>(w) / static_cast<float>(h));

    const uint32_t modelLoc = glGetUniformLocation(program.getId(), "model");
    const uint32_t viewLoc = glGetUniformLocation(program.getId(), "view");
    const uint32_t projLoc = glGetUniformLocation(program.getId(), "proj");

    glEnable(GL_DEPTH_TEST);

    const auto processMouse = [&window, &cam]() {
        static double lastXPos = 0;
        static double lastYPos = 0;
        static bool firstMouse = true;

        const auto [xPos, yPos] = Input::getMousePos(window.handle());
        if (firstMouse) {
            lastXPos = xPos;
            lastYPos = yPos;
            firstMouse = false;
        }

        float xOffset = xPos - lastXPos;
        float yOffset = yPos - lastYPos;
        lastXPos = xPos;
        lastYPos = yPos;

        const float sensitivity = 0.2f;
        cam.rotate(xOffset * sensitivity, -yOffset * sensitivity);
    };

    const auto processKeyboard = [&window, &cam](double dt) {
        float moveSpeed = 5.0f * dt;
        using D = Camera::Dir;
        const auto keyDown = std::bind(Input::isKeyDown, window.handle(), std::placeholders::_1);

        if (keyDown(GLFW_KEY_T)) {
            moveSpeed = 500.0f * dt;
        }

        if (keyDown(GLFW_KEY_W)) {
            cam.move<D::Front>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_A)) {
            cam.move<D::Left>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_S)) {
            cam.move<D::Back>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_D)) {
            cam.move<D::Right>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_SPACE)) {
            cam.move<D::Up>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_LEFT_SHIFT)) {
            cam.move<D::Down>(moveSpeed);
        }
    };

    double lastTime = 0;
    glfwSetInputMode(window.handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window.handle())) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            const double time = glfwGetTime();
            const double dt = time - lastTime;
            lastTime = time;

            processMouse();
            processKeyboard(dt);

            cam.update();
        }

        program.bind();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));

        // glBindTextureUnit(0, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window.handle());
        glfwPollEvents();
    }

    // glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwTerminate();
}
