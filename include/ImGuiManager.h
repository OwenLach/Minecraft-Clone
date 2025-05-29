#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

class ImGuiManager
{
public:
    ImGuiManager(GLFWwindow *window);
    ~ImGuiManager();
    void update();
    void render();
};