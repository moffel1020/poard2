#include "window.h"

Window::Window(uint32_t width, uint32_t height, const char* title)
    : glfwWindow(glfwCreateWindow(width, height, title, nullptr, nullptr)) {

    glfwSetWindowUserPointer(glfwWindow.get(), this);
    glfwMakeContextCurrent(glfwWindow.get());
    glfwSwapInterval(1);
    glfwSetKeyCallback(glfwWindow.get(), &Window::keyCallback);
    glfwSetCursorPosCallback(glfwWindow.get(), &Window::mousePosCallback);
    glfwSetMouseButtonCallback(glfwWindow.get(), &Window::mouseButtonCallback);
}
