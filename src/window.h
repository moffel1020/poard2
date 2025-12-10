#pragma once
#include "input.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <glad/gl.h>
#include <memory>

struct WindowSize {
    uint32_t w;
    uint32_t h;
};

class Window {
public:
    Window(uint32_t width, uint32_t height, const char* title);
    GLFWwindow* handle() const { return glfwWindow.get(); }

    WindowSize size() const {
        int32_t w, h;
        glfwGetWindowSize(glfwWindow.get(), &w, &h);
        return WindowSize{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
    }

    Input& getInput() { return input; }

private:
    struct DeleteGlfwWindow {
        void operator()(GLFWwindow* w) { glfwDestroyWindow(w); }
    };

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;

        const auto& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win.input.keyCallback(key, action);
    }

    std::unique_ptr<GLFWwindow, DeleteGlfwWindow> glfwWindow;
    Input input;
};
