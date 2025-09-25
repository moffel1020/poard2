#include "camera.h"
#include "shader.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include <array>
#include <fstream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

int main() {
    if (!glfwInit()) {
        std::cout << "failed to init glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "poard2", NULL, NULL);
    if (window == nullptr) {
        std::cout << "failed to create glfw window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        glfwTerminate();
        std::cout << "failed to init glad" << std::endl;
        return -1;
    }

    constexpr float initScreenWidth = 800.0f;
    constexpr float initScreenHeight = 600.0f;
    glViewport(0, 0, initScreenWidth, initScreenHeight);
    glfwSetFramebufferSizeCallback(
        window, [](GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); });

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

    // create vertex array
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    uint32_t vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    uint32_t ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load texture
    uint32_t texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, numChannels;
    uint8_t* data = stbi_load("res/textures/container.jpg", &width, &height, &numChannels, 0);
    if (!data) {
        throw std::runtime_error("failed to load image from disk");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
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

        double xPos;
        double yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
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
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.move<Dir::Front>(moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam.move<Dir::Left>(moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam.move<Dir::Back>(moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam.move<Dir::Right>(moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cam.move<Dir::Up>(moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cam.move<Dir::Down>(moveSpeed);
        }
    };

    double lastTime = 0;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();

        {
            const double time = glfwGetTime();
            const double dt = time - lastTime;
            lastTime = time;

            processMouse();
            processKeyboard(dt);

            cam.update();

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
}
