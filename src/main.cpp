#include "camera.h"
#include "imgui_wrapper.h"
#include "input.h"
#include "shader.h"
#include "shader_program.h"
#include "terrain_gen.h"
#include "util.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <array>
#include <fstream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <sstream>
#include <stb_image.h>

// needed so the glfw context doesnt get destroyed before the opengl resources are freed
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
    Input& input = window.getInput();

    input.lockCursor(true);
    bool enableGui = false;

    input.onKeyPressed(GLFW_KEY_E, [&]() {
        const bool locked = input.isCursorLocked();
        input.lockCursor(!locked);
        enableGui = locked;
    });

    input.onKeyPressed(GLFW_KEY_P, [&]() {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    });

    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        throw std::runtime_error("failed to initialize glad");
    }

    Gui::init(window.handle());

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
    glNamedBufferStorage(vbo, TerrainGen::getVertexBufferSize(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    TerrainGen terrainGen;
    GenConfig genConfig{};

    const auto indices = TerrainGen::genHeightIndices();
    uint32_t ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, TerrainGen::getIndexBufferSize(), indices.data(), GL_STATIC_DRAW);

    uint32_t vao;
    glCreateVertexArrays(1, &vao);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(vao, ebo);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    const std::string vertSrc = readFile("res/shaders/shader.vert");
    const std::string fragSrc = readFile("res/shaders/shader.frag");
    const ShaderProgram program({
        Shader(vertSrc, ShaderType::Vertex),
        Shader(fragSrc, ShaderType::Fragment),
    });

    const std::string skyboxVertSrc = readFile("res/shaders/skybox.vert");
    const std::string skyboxFragSrc = readFile("res/shaders/skybox.frag");
    const ShaderProgram skyboxProgram({
        Shader(skyboxVertSrc, ShaderType::Vertex),
        Shader(skyboxFragSrc, ShaderType::Fragment),
    });

    uint32_t rockTexture;
    glCreateTextures(GL_TEXTURE_2D, 1, &rockTexture);

    glTextureParameteri(rockTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(rockTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(rockTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(rockTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load("res/textures/rock.jpg", &width, &height, &numChannels, 0);
    if (!data) {
        throw std::runtime_error("failed to load image from disk");
    }

    glTextureStorage2D(rockTexture, 1, GL_RGB8, width, height);
    glTextureSubImage2D(rockTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(rockTexture);

    uint32_t grassTexture;
    glCreateTextures(GL_TEXTURE_2D, 1, &grassTexture);

    glTextureParameteri(grassTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(grassTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(grassTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(grassTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("res/textures/grass3.jpg", &width, &height, &numChannels, 0);
    if (!data) {
        throw std::runtime_error("failed to load image from disk");
    }

    glTextureStorage2D(grassTexture, 1, GL_RGB8, width, height);
    glTextureSubImage2D(grassTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(grassTexture);

    stbi_image_free(data);

    uint32_t skyboxTexture;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyboxTexture);
    glTextureStorage2D(skyboxTexture, 1, GL_RGBA8, 512, 512);

    glTextureParameteri(skyboxTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const std::string skyboxPath = "res/textures/skybox/";
    const std::vector<std::string> faces{
        skyboxPath + "px.png",
        skyboxPath + "nx.png",
        skyboxPath + "py.png",
        skyboxPath + "ny.png",
        skyboxPath + "pz.png",
        skyboxPath + "nz.png",
    };

    stbi_set_flip_vertically_on_load(false);
    for (int face = 0; face < 6; ++face) {
        int width, height, numChannels;
        uint8_t* data = stbi_load(faces[face].c_str(), &width, &height, &numChannels, 0);
        if (!data) {
            throw std::runtime_error("could not load cubemap face " + faces[face]);
        }

        glTextureSubImage3D(skyboxTexture, 0, 0, 0, face, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // cube vertices
    const std::array skyboxVertices{-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

    uint32_t skyboxVbo;
    glCreateBuffers(1, &skyboxVbo);
    glNamedBufferStorage(skyboxVbo, sizeof(skyboxVertices), skyboxVertices.data(), GL_DYNAMIC_STORAGE_BIT);

    uint32_t skyboxVao;
    glCreateVertexArrays(1, &skyboxVao);
    glVertexArrayVertexBuffer(skyboxVao, 0, skyboxVbo, 0, sizeof(float) * 3);
    glEnableVertexArrayAttrib(skyboxVao, 0);
    glVertexArrayAttribFormat(skyboxVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(skyboxVao, 0, 0);

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    Camera cam(static_cast<float>(w) / static_cast<float>(h));
    cam.setPosition({200000.0f, 400.0f, 200000.0f});

    const uint32_t modelLoc = glGetUniformLocation(program.handle(), "model");
    const uint32_t viewLoc = glGetUniformLocation(program.handle(), "view");
    const uint32_t projLoc = glGetUniformLocation(program.handle(), "proj");
    const uint32_t camPosLoc = glGetUniformLocation(program.handle(), "camPos");

    const uint32_t powerLoc = glGetUniformLocation(program.handle(), "heightPower");
    const uint32_t fogDistanceLoc = glGetUniformLocation(program.handle(), "fogDistance");
    const uint32_t scaleLoc = glGetUniformLocation(program.handle(), "heightScale");

    glEnable(GL_DEPTH_TEST);

    const auto processMouse = [&cam, &input]() {
        static double lastXPos = 0;
        static double lastYPos = 0;
        static bool firstMouse = true;

        if (!input.isCursorLocked()) {
            firstMouse = true;
            return;
        }

        const auto [xPos, yPos] = input.getMousePos();
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

    const auto processKeyboard = [&input, &cam](double dt) {
        float moveSpeed = 400.0f * dt;
        using D = Camera::Dir;

        if (input.isKeyDown(GLFW_KEY_T)) {
            moveSpeed = 1000.0f * dt;
        }

        if (input.isKeyDown(GLFW_KEY_W)) {
            cam.move<D::Front>(moveSpeed);
        }
        if (input.isKeyDown(GLFW_KEY_A)) {
            cam.move<D::Left>(moveSpeed);
        }
        if (input.isKeyDown(GLFW_KEY_S)) {
            cam.move<D::Back>(moveSpeed);
        }
        if (input.isKeyDown(GLFW_KEY_D)) {
            cam.move<D::Right>(moveSpeed);
        }
        if (input.isKeyDown(GLFW_KEY_SPACE)) {
            cam.move<D::Up>(moveSpeed);
        }
        if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
            cam.move<D::Down>(moveSpeed);
        }
    };

    double lastTime = 0;

    float heightScale = 200.0f;
    float heightPower = 1.0f;
    glm::vec2 fogDistance(700.0f, 2500.0f);

    glfwSetInputMode(window.handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

        Gui::startFrame();

        const auto camPos = cam.getPosition();
        const glm::ivec2 chunkPos(
            std::floor(camPos.x / TerrainGen::chunkSize), std::floor(camPos.z / TerrainGen::chunkSize));

        if (enableGui) {
            const glm::vec3 camPos = cam.getPosition();
            ImGui::Begin("Terrain settings");
            ImGui::Text("cam chunk x: %f z: %f", std::floor(camPos.x / TerrainGen::chunkSize),
                std::floor(camPos.z / TerrainGen::chunkSize));

            ImGui::SeparatorText("Render settings");
            ImGui::DragFloat("scale", &heightScale);
            ImGui::DragFloat("power", &heightPower, 0.1f, 0.5f, 10.0f);
            ImGui::DragFloat2("fog distance (min/max)", glm::value_ptr(fogDistance), 20.0f);

            ImGui::SeparatorText("Generation settings");
            if (ImGui::Button("reset")) {
                genConfig = GenConfig{};
            }

            ImGui::DragInt("grid size", reinterpret_cast<int*>(&genConfig.gridSize), 5.0f, 0, 100000.0f);
            ImGui::DragInt("octaves", reinterpret_cast<int*>(&genConfig.octaves), 0.5, 0.0f, 150.0f);
            ImGui::DragFloat("lacunarity", &genConfig.lacunarity, 0.05f);
            ImGui::DragFloat("gain", &genConfig.gain, 0.05f);

            if (ImGui::Button("generate")) {
                terrainGen.clearChunkCache();
                terrainGen.setConfig(genConfig);
                terrainGen.update(compProgram, vbo, chunkPos);
            }
            ImGui::End();
        }

        terrainGen.update(compProgram, vbo, chunkPos);

        program.bind();
        glUniform1f(scaleLoc, heightScale);
        glUniform1f(powerLoc, heightPower);
        glUniform2f(fogDistanceLoc, fogDistance.x, fogDistance.y);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));
        glUniform3fv(camPosLoc, 1, glm::value_ptr(cam.getPosition()));
        glBindTextureUnit(0, rockTexture);
        glBindTextureUnit(1, grassTexture);
        glBindVertexArray(vao);
        for (uint32_t i = 0; i < terrainGen.chunkCount; i++) {
            glDrawElementsBaseVertex(GL_TRIANGLES, TerrainGen::elemCount, GL_UNSIGNED_INT, 0,
                TerrainGen::chunkSize * TerrainGen::chunkSize * i);
        }

        glDepthFunc(GL_LEQUAL);
        skyboxProgram.bind();
        const glm::mat4 view2 = glm::mat4(glm::mat3(cam.getView()));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view2));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.getProj()));
        glBindTextureUnit(1, skyboxTexture);
        glBindVertexArray(skyboxVao);
        glDrawArrays(GL_TRIANGLES, 0, skyboxVertices.size());
        glDepthFunc(GL_LESS);

        Gui::endFrame();

        glfwSwapBuffers(window.handle());
        glfwPollEvents();
    }

    Gui::shutdown();

    glDeleteTextures(1, &rockTexture);
    glDeleteTextures(1, &grassTexture);
    glDeleteVertexArrays(1, &skyboxVao);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &skyboxVbo);
    glDeleteBuffers(1, &ebo);
}
