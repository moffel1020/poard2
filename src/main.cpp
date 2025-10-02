#include "camera.h"
#include "input.h"
#include "shader.h"
#include "stb_image.h"
#include "window.h"

#include <GLFW/glfw3.h>
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
        case GL_DEBUG_TYPE_OTHER:  return "OTHER";
        default:  return "";
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

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    constexpr float initScreenWidth = 800.0f;
    constexpr float initScreenHeight = 600.0f;
    Window window(initScreenWidth, initScreenHeight, "poard2");

    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        throw std::runtime_error("failed to initialize glad");
    }

    glViewport(0, 0, initScreenWidth, initScreenHeight);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&debugCallback, nullptr);

    glfwSetFramebufferSizeCallback(
        window.getHandle(), [](GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); });

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

    // create vertex buffer
    // clang-format off
    const std::array vertices = {
        // vertices          // texCoords
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // front top right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  // front bottom right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // front bottom left
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  // front top left 

         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // back top right
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // back bottom right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // back bottom left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // back top left 
    };

    const std::array indices = {
        // front
        0u, 1u, 3u,
        1u, 2u, 3u, 
        // back
        4u, 5u, 7u,
        5u, 6u, 7u, 
        // top
        // bot
        // left
        // right
    };
    // clang-format on

    uint32_t vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    uint32_t ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    uint32_t vao;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 5 * sizeof(float));
    glVertexArrayElementBuffer(vao, ebo);

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);

    // load texture
    uint32_t texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, numChannels;
    uint8_t* data = stbi_load("res/textures/container.jpg", &width, &height, &numChannels, 0);
    if (!data) {
        throw std::runtime_error("failed to load image from disk");
    }

    glTextureStorage2D(texture, 1, GL_RGB8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(texture);

    stbi_set_flip_vertically_on_load(true);
    stbi_image_free(data);

    // setup mvp matrices
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    Camera cam(initScreenWidth / initScreenHeight);

    const uint32_t modelLoc = glGetUniformLocation(shader.getId(), "model");
    const uint32_t viewLoc = glGetUniformLocation(shader.getId(), "view");
    const uint32_t projLoc = glGetUniformLocation(shader.getId(), "proj");

    glEnable(GL_DEPTH_TEST);

    const auto processMouse = [&window, &cam]() {
        static double lastXPos = 0;
        static double lastYPos = 0;
        static bool firstMouse = true;

        const auto [xPos, yPos] = Input::getMousePos(window.getHandle());
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
        const float moveSpeed = 2.0f * dt;
        using Dir = Camera::Dir;
        const auto keyPressed = std::bind(Input::isKeyPressed, window.getHandle(), std::placeholders::_1);

        if (keyPressed(GLFW_KEY_W)) {
            cam.move<Dir::Front>(moveSpeed);
        }
        if (keyPressed(GLFW_KEY_A)) {
            cam.move<Dir::Left>(moveSpeed);
        }
        if (keyPressed(GLFW_KEY_S)) {
            cam.move<Dir::Back>(moveSpeed);
        }
        if (keyPressed(GLFW_KEY_D)) {
            cam.move<Dir::Right>(moveSpeed);
        }
        if (keyPressed(GLFW_KEY_SPACE)) {
            cam.move<Dir::Up>(moveSpeed);
        }
        if (keyPressed(GLFW_KEY_LEFT_SHIFT)) {
            cam.move<Dir::Down>(moveSpeed);
        }
    };

    double lastTime = 0;
    glfwSetInputMode(window.getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window.getHandle())) {
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

        glfwSwapBuffers(window.getHandle());
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwTerminate();
}
