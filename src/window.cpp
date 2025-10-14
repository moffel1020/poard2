#include "window.h"
#include <stdexcept>

Window::Window(uint32_t width, uint32_t height, const char* title)
    : glfwWindow(glfwCreateWindow(width, height, title, nullptr, nullptr)) {
    
    if (!glfwWindow) {
        throw std::runtime_error("failed to create glfw window");
    }

    glfwSetWindowUserPointer(glfwWindow.get(), this);
    glfwMakeContextCurrent(glfwWindow.get());
    glfwSwapInterval(1);
    // glfwSetKeyCallback(glfwWindow.get(), &Window::keyCallback);
    // glfwSetCursorPosCallback(glfwWindow.get(), &Window::mousePosCallback);
    // glfwSetMouseButtonCallback(glfwWindow.get(), &Window::mouseButtonCallback);
    // glfwSetScrollCallback(glfwWindow.get(), &Window::scrollCallback);
}
