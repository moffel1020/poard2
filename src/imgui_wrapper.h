#pragma once
#include <GLFW/glfw3.h>
#include <iostream>

namespace Gui {

void init(GLFWwindow* window);
void startFrame();
void endFrame();
void shutdown();

}; // namespace Gui
