#pragma once
#include <GLFW/glfw3.h>

struct MousePos {
    float x;
    float y;
};

class Input {
public:
    static bool isKeyDown(GLFWwindow* w, int glfwKey) { return glfwGetKey(w, glfwKey) == GLFW_PRESS; }

    static bool isMousePressed(GLFWwindow* w, int glfwMouseButton) {
        return glfwGetMouseButton(w, glfwMouseButton) == GLFW_PRESS;
    }

    static MousePos getMousePos(GLFWwindow* w) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        return MousePos{static_cast<float>(x), static_cast<float>(y)};
    }
};
