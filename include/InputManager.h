#pragma once

#include "Camera.h"
#include <GLFW/glfw3.h>

class World;

class InputManager
{
public:
    InputManager(Camera &camera, GLFWwindow *window, World &world);
    void processKeyboard(GLFWwindow *window, float deltaTime);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

private:
    Camera &camera_;
    GLFWwindow *window_;
    World &world_;

    bool firstMouse_;
    float lastX_;
    float lastY_;

    void setInputCallbacks();
};