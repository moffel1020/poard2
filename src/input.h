#pragma once
#include <GLFW/glfw3.h>
#include <functional>

struct MousePos {
    float x;
    float y;
};

class Input {
public:
    using KeyCallback = std::function<void()>;

    bool isKeyDown(int glfwKey) const { return glfwGetKey(window, glfwKey) == GLFW_PRESS; }

    bool isMousePressed(int glfwMouseButton) const { return glfwGetMouseButton(window, glfwMouseButton) == GLFW_PRESS; }

    MousePos getMousePos() const {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return MousePos{static_cast<float>(x), static_cast<float>(y)};
    }

    void onKeyPressed(int key, KeyCallback&& cb) { keyCallbacks.emplace_back(std::make_pair(key, std::move(cb))); }

    void clearKeyCallbacks() { keyCallbacks.clear(); }

    bool isCursorLocked() { return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }

    void setCursorLock(bool locked) {
        if (locked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

private:
    friend class Window;

    void keyCallback(int key, int action) const {
        if (action != GLFW_PRESS) {
            return;
        }

        for (const auto& [cb_key, cb] : keyCallbacks) {
            if (key == cb_key) {
                cb();
            }
        }
    }

    Input(GLFWwindow* win) : window(win) {}

    GLFWwindow* window;
    std::vector<std::pair<int, KeyCallback>> keyCallbacks;
};
