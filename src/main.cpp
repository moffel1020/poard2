#include "camera.h"
#include "input.h"
#include "shader.h"
#include "shader_program.h"
#include "terrain_gen.h"
#include "util.h"
#include "window.h"


#include <GLFW/glfw3.h>
#include <fstream>
#include <functional>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <stb_image.h>

struct GlfwContext {
    GlfwContext() {
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize glfw");
        }
    }

    ~GlfwContext() { glfwTerminate(); }
};

int main() {
    GlfwContext ctx;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    Window window(1280, 720, "poard2");

    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        throw std::runtime_error("failed to initialize glad");
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&Util::debugCallback, nullptr);

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

    const std::string compSource = readFile("res/shaders/terrain.comp");
    const ShaderProgram compProgram({Shader(compSource, ShaderType::Compute)});

    uint32_t vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, Terrain::getVertexBufferSize(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    Terrain::genHeightmapDevice(compProgram, vbo);

    const auto indices = Terrain::genHeightIndicesHost();
    uint32_t ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, Terrain::getIndexBufferSize(), indices.data(), GL_STATIC_DRAW);

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

    const std::string vertSrc = readFile("res/shaders/shader.vert");
    const std::string fragSrc = readFile("res/shaders/shader.frag");
    const ShaderProgram program({
        Shader(vertSrc, ShaderType::Vertex),
        Shader(fragSrc, ShaderType::Fragment),
    });

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
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
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

    const auto processKeyboard = [&](double dt) {
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

        // if (keyDown(GLFW_KEY_P)) {
        //     Terrain::genHeightmapDevice(compProgram, vbo);
        //     std::cout << "done\n";
        // }
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
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));

        // glBindTextureUnit(0, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, Terrain::elemCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window.handle());
        glfwPollEvents();
    }

    // glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}
