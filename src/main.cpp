#include "camera.h"
#include "input.h"
#include "noise.h"
#include "shader.h"
#include "stb_image.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

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

std::vector<uint8_t> genHeightmap(int w, int h) {
    constexpr int gridSize = 40;
    constexpr int octaves = 12;
    constexpr float lacunarity = 2;
    constexpr float gain = 0.5;

    std::vector<uint8_t> image;
    image.resize(w * h * 3);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int index = (y * w + x) * 3;
            float val = 0;
            float freq = 1;
            float amp = 1;

            for (int i = 0; i < octaves; i++) {
                val += Noise::perlin(x * freq / gridSize, y * freq / gridSize) * amp;
                freq *= lacunarity;
                amp *= gain;
            }

            val = std::clamp(val * 1.2f, -1.0f, 1.0f);

            int color = (int)(((val + 1.0f) * 0.5f) * 255);
            // this is dumb
            image[index] = color;
            image[index + 1] = color;
            image[index + 2] = color;
        }
    }

    return image;
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
    const Shader shader(vertSrc, fragSrc);

    // clang-format off
    const std::array vertices = {
        // vertices          // colors           // texCoords
         0.5f,  0.5f,  0.5f,  0.2f, 0.5f, 0.2f,  1.0f, 1.0f,            // front top right
         0.5f, -0.5f,  0.5f,  0.2f, 0.5f, 0.2f,  1.0f, 0.0f,            // front bottom right
        -0.5f, -0.5f,  0.5f,  0.2f, 0.5f, 0.2f,  0.0f, 0.0f,            // front bottom left
        -0.5f,  0.5f,  0.5f,  0.2f, 0.5f, 0.2f,  0.0f, 1.0f,            // front top left 
    };

    const std::array indices = {
        0u, 1u, 3u,
        1u, 2u, 3u, 
    };
    // clang-format on

    uint32_t vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    uint32_t ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    uint32_t vao;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 8 * sizeof(float));
    glVertexArrayElementBuffer(vao, ebo);

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glEnableVertexArrayAttrib(vao, 2);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);
    glVertexArrayAttribBinding(vao, 2, 0);

    uint32_t texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // int width, height, numChannels;
    // uint8_t* data = stbi_load("res/textures/container.jpg", &width, &height, &numChannels, 0);
    // if (!data) {
    //     throw std::runtime_error("failed to load image from disk");
    // }

    // glTextureStorage2D(texture, 1, GL_RGB8, width, height);
    // glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    // glGenerateTextureMipmap(texture);

    constexpr int hWidth = 512;
    constexpr int hHeight = 512;
    const auto hTexture = genHeightmap(hWidth, hHeight);

    glTextureStorage2D(texture, 1, GL_RGB8, hWidth, hHeight);
    glTextureSubImage2D(texture, 0, 0, 0, hWidth, hHeight, GL_RGB, GL_UNSIGNED_BYTE, hTexture.data());
    glGenerateTextureMipmap(texture);

    // stbi_set_flip_vertically_on_load(true);
    // stbi_image_free(data);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // model = glm::translate(model, glm::vec3(0.0f, 1000.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(10000.0f));
    Camera cam(static_cast<float>(w) / static_cast<float>(h));

    const uint32_t modelLoc = glGetUniformLocation(shader.getId(), "model");
    const uint32_t viewLoc = glGetUniformLocation(shader.getId(), "view");
    const uint32_t projLoc = glGetUniformLocation(shader.getId(), "proj");

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
        using Dir = Camera::Dir;
        const auto keyDown = std::bind(Input::isKeyDown, window.handle(), std::placeholders::_1);

        if (keyDown(GLFW_KEY_T)) {
            moveSpeed = 500.0f * dt;
        }

        if (keyDown(GLFW_KEY_W)) {
            cam.move<Dir::Front>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_A)) {
            cam.move<Dir::Left>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_S)) {
            cam.move<Dir::Back>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_D)) {
            cam.move<Dir::Right>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_SPACE)) {
            cam.move<Dir::Up>(moveSpeed);
        }
        if (keyDown(GLFW_KEY_LEFT_SHIFT)) {
            cam.move<Dir::Down>(moveSpeed);
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

        shader.bind();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));

        glBindTextureUnit(0, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window.handle());
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwTerminate();
}
